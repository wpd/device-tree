/******************************************************************************
 *
 *       XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION "AS IS"
 *       AS A COURTESY TO YOU, SOLELY FOR USE IN DEVELOPING PROGRAMS AND
 *       SOLUTIONS FOR XILINX DEVICES. BY PROVIDING THIS DESIGN, CODE,
 *       OR INFORMATION AS ONE POSSIBLE IMPLEMENTATION OF THIS FEATURE,
 *       APPLICATION OR STANDARD, XILINX IS MAKING NO REPRESENTATION
 *       THAT THIS IMPLEMENTATION IS FREE FROM ANY CLAIMS OF INFRINGEMENT,
 *       AND YOU ARE RESPONSIBLE FOR OBTAINING ANY RIGHTS YOU MAY REQUIRE
 *       FOR YOUR IMPLEMENTATION. XILINX EXPRESSLY DISCLAIMS ANY
 *       WARRANTY WHATSOEVER WITH RESPECT TO THE ADEQUACY OF THE
 *       IMPLEMENTATION, INCLUDING BUT NOT LIMITED TO ANY WARRANTIES OR
 *       REPRESENTATIONS THAT THIS IMPLEMENTATION IS FREE FROM CLAIMS OF
 *       INFRINGEMENT, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *       FOR A PARTICULAR PURPOSE.
 *
 *       (c) Copyright 2007 Xilinx Inc.
 *       All rights reserved. 
* This program is free software; you can redistribute it and/or modify it 
* under the terms of the GNU General Public License as published by the 
* Free Software Foundation; either version 2 of the License, or (at your 
* option) any later version. 
*
* You should have received a copy of the GNU General Public License 
* along with this program; if not, write to the Free Software 
* Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA 
 *
 ******************************************************************************/

/******************************************************************************/
/**
 * @file xlnx_udc.c
 *
 * This file contains the Linux USB peripheral driver.
 *
 * @note	None.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- ------------------------------------------------------------------
 * 1.00a hvm  4/20/07 First release
 *
 * </pre>
 ******************************************************************************/

/***************************** Include Files *********************************/
#include<linux/version.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,18) 
#include <linux/platform_device.h>
#else
#include <linux/config.h>
#endif
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/ioport.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/smp_lock.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/timer.h>
#include <linux/list.h>
#include <linux/interrupt.h>
#include <linux/proc_fs.h>
#include <linux/moduleparam.h>
#include <linux/device.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,18) 
#include <linux/usb/ch9.h>
#else
#include <linux/usb_ch9.h>
#endif
#include <linux/usb_gadget.h>

#include <asm/byteorder.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/system.h>
#include <asm/unaligned.h>
#include <linux/seq_file.h>

#include "xusb.h"
#include "xusb_cp9.h"
#include "xlnx_udc.h"

#include "gadget_chips.h"

/************************** Constant Definitions *****************************/

#define  PACKET_TRACE		1
#define USB_EP_BUFF1_MASK	XUSB_BUFFREADY_EP0_BUFF_MASK
#define USB_EP_BUFF2_MASK	0x00000100

/**************************** Type Definitions *******************************/

/*
 * Write
 */
#define DRIVER_VERSION  "20 April 2008"

static const char driver_name [] = "xlnx_udc";
static const char ep0name[] = "ep0";

union setup {
         u8                      raw[8];
         struct usb_ctrlrequest  r;
 };

/*
 * Control end point configuration
 */
static struct usb_endpoint_descriptor
config_bulk_out_desc = {
	.bLength =		USB_DT_ENDPOINT_SIZE,
	.bDescriptorType =	USB_DT_ENDPOINT,

	.bEndpointAddress =	USB_DIR_OUT,
	.bmAttributes =		USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize =	__constant_cpu_to_le16(0x40),
};

/************************** Function Prototypes *******************************/

/*-------------------------------------------------------------------------*/


static const char debug_filename[] = "driver/udc";

static int proc_udc_show(struct seq_file *s, void *unused)
{
        struct xusb_udc *udc = s->private;

        seq_printf(s, "%s: version %s\n", driver_name, DRIVER_VERSION);

        seq_printf(s, "vbus %s, pullup %s, %s powered%s, gadget %s\n\n",
                udc->vbus ? "present" : "off",
                udc->enabled
                        ? (udc->vbus ? "active" : "enabled")
                        : "disabled",
                udc->selfpowered ? "self" : "VBUS",
                udc->suspended ? ", suspended" : "",
                udc->driver ? udc->driver->driver.name : "(none)");
        return 0;
}

static int proc_udc_open(struct inode *inode, struct file *file)
{
        return single_open(file, proc_udc_show, PDE(inode)->data);
}

static struct file_operations proc_ops = {
        .open           = proc_udc_open,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};

static void create_debug_file(struct xusb_udc *udc)
{
        struct proc_dir_entry *pde;

        pde = create_proc_entry (debug_filename, 0, NULL);
        udc->pde = pde;
        if (pde == NULL)
                return;

        pde->proc_fops = &proc_ops;
        pde->data = udc;
}
/*-------------------------------------------------------------------------*/

static void done(struct xusb_ep *ep, struct xusb_request *req, int status)
{
        unsigned        stopped = ep->stopped;

        list_del_init(&req->queue);

        if (req->req.status == -EINPROGRESS)
                req->req.status = status;
        else
                status = req->req.status;

        if (status && status != -ESHUTDOWN)
                VDBG("%s done %p, status %d\n", ep->ep.name, req, status);

        ep->stopped = 1;

	spin_unlock(&ep->udc->lock);
        req->req.complete(&ep->ep, &req->req);
	spin_lock(&ep->udc->lock);

        ep->stopped = stopped;
}

/*-------------------------------------------------------------------------*/


/* pull OUT packet data from the endpoint's fifo */
static int read_fifo (struct xusb_ep *ep, struct xusb_request *req)
{
        u8              *buf;
        unsigned  is_short,count, bufferspace; 	unsigned char Bufoffset;
	u32 BufRdy;
	u8 two_pkts = 0;
	spin_unlock(&ep->udc->lock);
	BufRdy = XUsb_mReadReg(ep->udc->usb.Config.BaseAddress, 
				XUSB_BUFFREADY_OFFSET);
	spin_lock(&ep->udc->lock);

	if ((ep->udc->usb.DeviceConfig.Ep[ep->epnumber].Buffer0Ready ==	1) &&
		(ep->udc->usb.DeviceConfig.Ep[ep->epnumber].Buffer1Ready == 1)){
		VDBG("%s: Packet NOT ready!\n", __FUNCTION__);
		return -EINVAL;
	}
top:
	if (ep->udc->usb.DeviceConfig.Ep[ep->epnumber].CurBufNum == 0){
		Bufoffset = 0x8;
	}else {
		Bufoffset = 0x0C;
	}
	count = XUsb_mReadReg(ep->udc->usb.Config.BaseAddress, 
				(ep->udc->usb.EndPointOffset[ep->epnumber] +
					 Bufoffset));
	if ((ep->udc->usb.DeviceConfig.Ep[ep->epnumber].Buffer0Ready ==	0) &&
		(ep->udc->usb.DeviceConfig.Ep[ep->epnumber].Buffer1Ready == 0)){
		two_pkts = 1;
	}

	VDBG("curbufnum is %d  and buf0rdy is %d, buf1rdy is %d bufrdy = %x\n", 
		ep->udc->usb.DeviceConfig.Ep[ep->epnumber].CurBufNum,
		ep->udc->usb.DeviceConfig.Ep[ep->epnumber].Buffer0Ready,
		ep->udc->usb.DeviceConfig.Ep[ep->epnumber].Buffer1Ready,
		BufRdy);

	buf = req->req.buf + req->req.actual;
	prefetchw (buf);
	bufferspace = req->req.length - req->req.actual;
	
        req->req.actual += min(count, bufferspace);
	is_short = (count < ep->ep.maxpacket);

	if (count != 0){
		if (unlikely(bufferspace == 0)) {
			/* this happens when the driver's buffer
			 * is smaller than what the host sent.
			 * discard the extra data.
			 */
			if (req->req.status != -EOVERFLOW)
				VDBG("%s overflow %d\n", ep->ep.name, count);
			req->req.status = -EOVERFLOW;
		} else{
		if (XUsb_EpDataRecv(&ep->udc->usb, ep->epnumber, buf, count) ==
			XST_SUCCESS){
			VDBG("read %s, %d bytes%s req %p %d/%d\n",
			      ep->ep.name, count,
			      is_short ? "/S" : "", req, req->req.actual, 
				req->req.length);
				bufferspace -= 	count;	
			/* completion */
			if (is_short || req->req.actual == req->req.length) {
				done(ep, req, 0);
				return 1;
			}

			if (two_pkts){
				two_pkts = 0;
				goto top;
			}
		
		} else {
			VDBG("rcv fail..curbufnum is %d  and buf0rdy is %d, buf1rdy is %d\n", 
				ep->udc->usb.DeviceConfig.Ep[ep->epnumber].CurBufNum,
				ep->udc->usb.DeviceConfig.Ep[ep->epnumber].Buffer0Ready,
				ep->udc->usb.DeviceConfig.Ep[ep->epnumber].Buffer1Ready);
			req->req.actual -= min(count, bufferspace);
			return -EINVAL;
		}
		}
	}else {
		return -EINVAL;
	}
	
	return 0;
}

/* load fifo for an IN packet */
static int write_fifo(struct xusb_ep *ep, struct xusb_request *req)
{
        u8 *buf;
	u32 max;
	unsigned length;
	int is_last, is_short = 0;

	max = le16_to_cpu(ep->desc->wMaxPacketSize);

	if (req){	
		buf = req->req.buf + req->req.actual;
		prefetch(buf);
		length = req->req.length - req->req.actual;
	} else {
		buf = NULL;
		length = 0;
	}
	
	length = min(length, max);
        if (XUsb_EpDataSend(&ep->udc->usb, ep->epnumber, buf, length) == 	
       			XST_FAILURE){
		buf = req->req.buf - req->req.actual;
		VDBG("Send failure\n"); 
       		return 0;
	}
	else {
		req->req.actual += length;
	
		if (unlikely(length != max))
			is_last = is_short = 1;
		else {
			if (likely(req->req.length != req->req.actual)
			    || req->req.zero)
				is_last = 0;
			else
				is_last = 1;
		}
		VDBG("%s: wrote %s %d bytes%s%s %d left %p\n", __FUNCTION__,
		      ep->ep.name, length,
		      is_last ? "/L" : "", is_short ? "/S" : "",
		      req->req.length - req->req.actual, req);

		if (is_last) {
			done(ep, req, 0);
			return 1;
		}
	}
	return 0;
}


static void nuke(struct xusb_ep *ep, int status)
{
        struct xusb_request *req;

        while (!list_empty(&ep->queue)) {
                req = list_entry(ep->queue.next, struct xusb_request, queue);
                done(ep, req, status);
        }
}

/***************************** Endpoint realated functions*********************/
static int xusb_ep_set_halt(struct usb_ep *_ep, int value)
{
        struct xusb_ep  *ep = container_of(_ep, struct xusb_ep, ep);
	unsigned long flags;
	u32 EpCfgReg;
        if (!_ep || (!ep->desc && ep->epnumber != 0))
                return -EINVAL;

	spin_lock_irqsave(&ep->udc->lock, flags);

        if (ep->is_in && (!list_empty(&ep->queue)) && value){
		spin_unlock_irqrestore(&ep->udc->lock, flags);
                return -EAGAIN;
	}
        else {
                VDBG("halt %s\n", ep->ep.name);
        }
	if (value){
		XUsb_EpStall(&(ep->udc->usb),ep->epnumber);
		ep->stopped = 1;
	}else {
		
		ep->stopped = 0;
		XUsb_EpUnstall(&(ep->udc->usb),ep->epnumber);
		EpCfgReg = XUsb_mReadReg(ep->udc->usb.Config.BaseAddress,
				ep->udc->usb.EndPointOffset[ep->epnumber]);
		if (ep->epnumber != 0){
			EpCfgReg &= ~XUSB_EP_CFG_DATA_TOGGLE_MASK;
			XUsb_mWriteReg(ep->udc->usb.Config.BaseAddress, 
				ep->udc->usb.EndPointOffset[ep->epnumber],
			       EpCfgReg);

		}
	}

	spin_unlock_irqrestore(&ep->udc->lock, flags);
        return 0;
}

static int xusb_ep_enable(struct usb_ep *_ep,
				const struct usb_endpoint_descriptor *desc)
{
        struct xusb_ep  *ep = container_of(_ep, struct xusb_ep, ep);
        struct xusb_udc *dev = ep->udc;
        u32             tmp;
        u8		eptype = 0;
	unsigned long flags;
	/*
 	 * The chekc for _ep->name == ep0name is not done as this enable i used for 
	 * enabling ep0 also. In other gadget drivers, this ep name is not used.
	 */
	if (!_ep || !desc || ep->desc || desc->bDescriptorType != USB_DT_ENDPOINT){
			VDBG("first check fails \n");
			return -EINVAL;
	}

        if (!dev->driver || dev->gadget.speed == USB_SPEED_UNKNOWN) {
                DBG("bogus device state\n");
                return -ESHUTDOWN;
        }

        
	ep->is_in = ((desc->bEndpointAddress & USB_DIR_IN) != 0);
	/* The address of the endpoint is encoded as follows:
		Bit 3...0: The endpoint number
		Bit 6...4: Reserved, reset to zero
		Bit 7: Direction, ignored for
		control endpoints
			0 = OUT endpoint
			1 = IN endpoint */
	ep->epnumber = (desc->bEndpointAddress & 0x0f);
	ep->stopped = 0;
        ep->desc = desc;
        tmp = desc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;
	
	spin_lock_irqsave(&ep->udc->lock, flags);
	ep->ep.maxpacket = le16_to_cpu (desc->wMaxPacketSize);

        switch (tmp) {
	         case USB_ENDPOINT_XFER_CONTROL:
	                 DBG("only one control endpoint\n");
			/* NON- ISO */
	                 eptype = 0;
			 spin_unlock_irqrestore(&ep->udc->lock, flags);
	                 return -EINVAL;
	         case USB_ENDPOINT_XFER_INT:
			/* NON- ISO */
			eptype = 0;
	                 if (ep->ep.maxpacket > 64)
	                         goto bogus_max;
	                 break;
	         case USB_ENDPOINT_XFER_BULK:
			/* NON- ISO */
			eptype = 0;
	                 switch (ep->ep.maxpacket) {
	                 case 8:
	                 case 16:
	                 case 32:
	                 case 64:
			 case 512:
	                         goto ok;
	                 }
	 bogus_max:
	                 DBG("bogus maxpacket %d\n", ep->ep.maxpacket);
			 spin_unlock_irqrestore(&ep->udc->lock, flags);
	                 return -EINVAL;
	         case USB_ENDPOINT_XFER_ISOC:
			/* ISO */
			eptype = 1; 
			ep->is_iso = 1;
	                 break;
	         }
	
 ok:	ep->udc->usb.DeviceConfig.Ep[ep->epnumber].OutIn = ep->is_in;
	ep->udc->usb.DeviceConfig.Ep[ep->epnumber].EpType = eptype;
	ep->udc->usb.DeviceConfig.Ep[ep->epnumber].Size = ep->ep.maxpacket;
	ep->udc->usb.DeviceConfig.Ep[ep->epnumber].RamBase = ep->rambase;
	ep->udc->usb.DeviceConfig.Ep[ep->epnumber].Buffer0Ready = 0;
	ep->udc->usb.DeviceConfig.Ep[ep->epnumber].Buffer1Ready = 0;
	ep->udc->usb.DeviceConfig.Ep[ep->epnumber].CurBufNum = 0;

	XUsb_EpConfigure(&dev->usb, ep->epnumber, 
			(&dev->usb.DeviceConfig.Ep[ep->epnumber]));
	VDBG("Enable Endpoint %d max pkt is %d\n",ep->epnumber,
			ep->udc->usb.DeviceConfig.Ep[ep->epnumber].Size);

        /*
         * Enable the End point.
         */
	XUsb_EpEnable(&(dev->usb), ep->epnumber);

	if (ep->epnumber != 0)
		ep->udc->usb.DeviceConfig.Ep[ep->epnumber].RamBase <<= 2;
	if ((ep->epnumber != 0) && (ep->is_in == 0)){
	XUsb_mWriteReg(ep->udc->usb.Config.BaseAddress,
		XUSB_BUFFREADY_OFFSET, 1 << ep->epnumber);
	
	dev->usb.DeviceConfig.Ep[ep->epnumber].Buffer0Ready = 1;

	XUsb_mWriteReg(dev->usb.Config.BaseAddress,
		XUSB_BUFFREADY_OFFSET, (1 <<
		(ep->epnumber + XUSB_STATUS_EP_BUFF2_SHIFT)));

	dev->usb.DeviceConfig.Ep[ep->epnumber].Buffer1Ready = 1;
	}

	spin_unlock_irqrestore(&ep->udc->lock, flags);

        return 0;
}

static int xusb_ep_disable (struct usb_ep * _ep)
{
        struct xusb_ep  *ep = container_of(_ep, struct xusb_ep, ep);
        unsigned long   flags;

        if (ep == &ep->udc->ep[0]){
		VDBG("Ep0 disable called \n");
                return -EINVAL;
	}
        spin_lock_irqsave(&ep->udc->lock, flags);

        nuke(ep, -ESHUTDOWN);

        /* restore the endpoint's pristine config */
        ep->desc = NULL;
	/* The address of the endpoint is encoded as follows:
		Bit 3...0: The endpoint number
		Bit 6...4: Reserved, reset to zero
		Bit 7: Direction, ignored for
		control endpoints
			0 = OUT endpoint
			1 = IN endpoint */
	VDBG("USB Ep %d disable \n ",ep->epnumber);	

	XUsb_EpDisable(&(ep->udc->usb), ep->epnumber);

        spin_unlock_irqrestore(&ep->udc->lock, flags);
        return 0;
}

/*
 * Write the calls for configuring the end points
 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,14) 
static struct usb_request *xusb_ep_alloc_request(struct usb_ep *_ep, 
						gfp_t gfp_flags)
#else
static struct usb_request *xusb_ep_alloc_request(struct usb_ep *_ep, 
						int gfp_flags)
#endif
{
        struct xusb_request *req;

        req = kmalloc(sizeof *req, gfp_flags);
        if (!req)
                return NULL;

	memset(req, 0, sizeof *req);
        INIT_LIST_HEAD(&req->queue);
        return &req->req;
}
static void xusb_ep_free_request(struct usb_ep *_ep, struct usb_request *_req)
{
        struct xusb_request *req;

        req = container_of(_req, struct xusb_request, req);
        WARN_ON(!list_empty(&req->queue));
        kfree(req);
}
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,14) 
static void *xusb_ep_alloc_buffer(struct usb_ep *_ep, unsigned bytes, 
				dma_addr_t *dma, gfp_t gfp_flags)
#else
static void *xusb_ep_alloc_buffer(struct usb_ep *_ep, unsigned bytes, 
				dma_addr_t *dma, int gfp_flags)
#endif
{
	struct xusb_ep          *ep;
	struct xusb_udc         *udc;
	void *buf;
	char *retvalue;
	ep = container_of(_ep, struct xusb_ep, ep);
	udc = ep->udc;
	if (ep->epnumber == 0){
		buf = (void *)((udc->usb.DeviceConfig.Ep[ep->epnumber].RamBase << 2) +
				 udc->usb.Config.BaseAddress);
		return buf;	
	}

	retvalue = kmalloc(bytes, gfp_flags);
	return (retvalue);
}

static void xusb_ep_free_buffer(struct usb_ep *ep, void *buf, 
				dma_addr_t dma, unsigned bytes)
{
	struct xusb_ep          *_ep;
	_ep = container_of(ep, struct xusb_ep, ep);
	if (_ep->epnumber != 0)
	        kfree(buf);
}
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,14) 
static int xusb_ep_queue(struct usb_ep *_ep, struct usb_request *_req, gfp_t gfp_flags)
#else
static int xusb_ep_queue(struct usb_ep *_ep, struct usb_request *_req, int gfp_flags)
#endif
{
        struct xusb_request     *req;
        struct xusb_ep          *ep;
        struct xusb_udc         *dev;
        unsigned long           flags;

        req = container_of(_req, struct xusb_request, req);
        ep = container_of(_ep, struct xusb_ep, ep);
	dev = ep->udc;
        if (!_req || !_req->complete
                        || !_req->buf || !list_empty(&req->queue)) {
                DBG("invalid request\n");
                return -EINVAL;
        }

        if (!_ep || (!ep->desc && ep->ep.name != ep0name)) {
                DBG("invalid ep\n");
                return -EINVAL;
        }
        if (!dev || !dev->driver || dev->gadget.speed == USB_SPEED_UNKNOWN) {
                DBG("%s, bogus device state %p\n", 
				__FUNCTION__, dev->driver);
                return -ESHUTDOWN;
        }

        spin_lock_irqsave(&dev->lock, flags);

        _req->status = -EINPROGRESS;
        _req->actual = 0;

        /* try to kickstart any empty and idle queue */
//        if (list_empty(&ep->queue) && !ep->stopped) {
        if (list_empty(&ep->queue)) {
              if (ep->epnumber == 0){
			XUsb_mWriteReg(dev->usb.Config.BaseAddress,
				      XUSB_EP_BUF0COUNT_OFFSET, req->req.length);
	
			XUsb_mWriteReg(dev->usb.Config.BaseAddress,
				       XUSB_BUFFREADY_OFFSET, 1);
			req = 0;
		}else {
			
               		if (ep->is_in){
				VDBG("write_fifo called from queue\n");
                     		if (write_fifo(ep, req) == 1)
					req = 0;
			} else {
				VDBG("read_fifo called from queue\n");
				if (read_fifo(ep, req) == 1)
					req = 0;
			}
		}
        }

        if (req != 0)
                list_add_tail (&req->queue, &ep->queue);
        spin_unlock_irqrestore(&dev->lock, flags);
        return 0;
}

static int xusb_ep_dequeue(struct usb_ep *_ep, struct usb_request *_req)
{
        struct xusb_ep  *ep;
        struct xusb_request     *req;
	unsigned long flags;

        ep = container_of(_ep, struct xusb_ep, ep);
        if (!_ep || ep->ep.name == ep0name)
                return -EINVAL;
	
	spin_lock_irqsave(&ep->udc->lock, flags);
        /* make sure it's actually queued on this endpoint */
        list_for_each_entry (req, &ep->queue, queue) {
                if (&req->req == _req)
                        break;
        }
        if (&req->req != _req){
		spin_unlock_irqrestore(&ep->udc->lock, flags);
                return -EINVAL;
	}

        done(ep, req, -ECONNRESET);
	spin_unlock_irqrestore(&ep->udc->lock, flags);

        return 0;
}



/** 
 *  This controller doesn't have fifos
 */
static int xusb_fifo_status(struct usb_ep *_ep)
{
	/*
	 * Do nothing
	 */
	return 0;
}

/** 
 *  This controller doesn't have fifos 
 */
static void xusb_fifo_flush(struct usb_ep *_ep)
{
}

 static struct usb_ep_ops xusb_ep_ops = {
         .enable         = xusb_ep_enable,
         .disable        = xusb_ep_disable,

         .alloc_request  = xusb_ep_alloc_request,
         .free_request   = xusb_ep_free_request,

         .alloc_buffer   = xusb_ep_alloc_buffer,
         .free_buffer    = xusb_ep_free_buffer,

         .queue          = xusb_ep_queue,
         .dequeue        = xusb_ep_dequeue,
         .set_halt       = xusb_ep_set_halt,
 	 .fifo_status    = xusb_fifo_status,
	 .fifo_flush     = xusb_fifo_flush,

 };

static int xusb_get_frame(struct usb_gadget *gadget)
{

        struct xusb_udc *udc = to_udc(gadget);
        unsigned long           flags;
        int                     retval;

        if (!gadget)
               return -ENODEV;

        if (!udc->clocked)
                return -EINVAL;

        local_irq_save(flags);
        retval = XUsb_GetFrameNum(&udc->usb);
        local_irq_restore(flags);
        return retval;
}

static int xusb_wakeup(struct usb_gadget *gadget)
{

        DBG("%s\n", __FUNCTION__ );

	/*
	 * Currently, the device does not support a remote wakeup
	 * provision.
	 */
        return -ENOTSUPP;
}

static int xusb_ioctl(struct usb_gadget *gadget, unsigned code,
			unsigned long param)
{
        struct xusb_udc *udc = to_udc(gadget);
        u8* BufPtr;
        BufPtr = (u8 *)param;

	if ((code == TEST_J) || (code == TEST_K) ||
		(code == TEST_SE0_NAK) || (code == TEST_PKT)) {
		XUsb_SetTestMode(&udc->usb, code, BufPtr);
	}
	else {
		return EINVAL;
	}

	return 0;
}

static const struct usb_gadget_ops xusb_udc_ops = {
        .get_frame              = xusb_get_frame,
        .wakeup                 = xusb_wakeup,
	.ioctl			= xusb_ioctl,
};


/*-------------------------------------------------------------------------*/
static struct xusb_udc controller = {
         .gadget = {
                 .ops = &xusb_udc_ops,
                 .ep0 = &controller.ep[0].ep,
                 .speed =  USB_SPEED_HIGH,
                 .is_dualspeed = 1,
                 .is_otg = 0,
		 .is_a_peripheral = 0,
		 .b_hnp_enable = 0,
		 .a_hnp_support = 0,
		 .a_alt_hnp_support = 0,
                 .name = driver_name,
                 .dev = {
                         .bus_id = "xlnx_udc",
                 },
         },
         .ep[0] = {
                 .ep = {
                         .name   = ep0name,
                         .ops    = &xusb_ep_ops,
                 },
                 .udc            = &controller,
                 .epnumber = 0,
		 .rambase = 0x40,
         },
         .ep[1] = {
                 .ep = {
                         .name   = "ep-a",
                         .ops    = &xusb_ep_ops,
                 },
                 .udc            = &controller,
                 .epnumber = 1,
		 .rambase = 0x200,
         },
         .ep[2] = {
                 .ep = {
                         .name   = "ep-b",
                         .ops    = &xusb_ep_ops,
                 },
                 .udc            = &controller,
                 .epnumber = 2,
		 .rambase = 0x400,
         },
         .ep[3] = {
                 .ep = {
                         .name   = "ep-c",
                         .ops    = &xusb_ep_ops,
                 },
                 .udc            = &controller,
                 .epnumber = 3,
		 .rambase = 0x600,
         },
         .ep[4] = {
                 .ep = {
                         .name   = "ep-d",
                         .ops    = &xusb_ep_ops,
                 },
                 .udc            = &controller,
                 .epnumber = 4,
		 .rambase = 0x800,
         },
         .ep[5] = {
                 .ep = {
                         .name   = "ep-e",
                         .ops    = &xusb_ep_ops,
                 },
                 .udc            = &controller,
                 .epnumber = 5,
		 .rambase = 0xa00,
         },
         .ep[6] = {
                 .ep = {
                         .name   = "ep-f",
                         .ops    = &xusb_ep_ops,
                 },
                 .udc            = &controller,
                 .epnumber = 6,
		 .rambase = 0xb00,
         },
         .ep[7] = {
                 .ep = {
                         .name   = "ep-g",
                         .ops    = &xusb_ep_ops,
                 },
                 .udc            = &controller,
                 .epnumber = 7,
		 .rambase = 0xd00,
         },
 };

/* reinit == restore inital software state */
static void udc_reinit(struct xusb_udc *udc)
{
        u32 i;

        INIT_LIST_HEAD(&udc->gadget.ep_list);
        INIT_LIST_HEAD(&udc->gadget.ep0->ep_list);
	udc->usb.DeviceConfig.NumEndpoints = 8;
	udc->req_pending = 0;
        for (i = 0; i < XUSB_MAX_ENDPOINTS; i++) {
                struct xusb_ep *ep = &udc->ep[i];

                if (i != 0)
                        list_add_tail(&ep->ep.ep_list, &udc->gadget.ep_list);
                ep->desc = NULL;
		ep->stopped = 0;
		udc->usb.DeviceConfig.Ep[ep->epnumber].RamBase = ep->rambase;
		XUsb_EpConfigure(&udc->usb, i, &udc->usb.DeviceConfig.Ep[i]);
		if (i != 0)
			udc->usb.DeviceConfig.Ep[ep->epnumber].RamBase <<= 2;
		udc->usb.DeviceConfig.CurrentConfiguration = 0;
                // initialiser one queue per endpoint
                INIT_LIST_HEAD(&ep->queue);
        }

}
static void stop_activity(struct xusb_udc *udc)
{
        struct usb_gadget_driver *driver = udc->driver; 
        int i;

        if (udc->gadget.speed == USB_SPEED_UNKNOWN)
                driver = NULL;
        udc->gadget.speed = USB_SPEED_HIGH;

        for (i = 0; i < XUSB_MAX_ENDPOINTS; i++) {
                struct xusb_ep *ep = &udc->ep[i];
		ep->stopped = 1;
                nuke(ep, -ESHUTDOWN);
        }
        if (driver){
		spin_unlock(&udc->lock);
                driver->disconnect(&udc->gadget);
		spin_lock(&udc->lock);
	}

        udc_reinit(udc);
}
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,18) 
static irqreturn_t xusb_udc_irq (int irq, void *_udc)
#else
static irqreturn_t xusb_udc_irq (int irq, void *_udc, struct pt_regs *r)
#endif
 {
        struct xusb_udc         *udc = _udc;
	XUsb *UsbInstPtr;
	u32 IntrStatus;
	XUsb_EpConfig *Ep;
	u32 BufIntr;

	spin_lock(&(udc->lock));

	UsbInstPtr = (XUsb *) &udc->usb;


	/*
	 * Read the Interrupt Status Register.
	 */
	IntrStatus = XUsb_mReadReg(UsbInstPtr->Config.BaseAddress,
				   XUSB_STATUS_OFFSET);

	/*
	 * Call the handler for the event interrupt.
	 */
	if (IntrStatus & XUSB_STATUS_INTR_EVENT_MASK) {

		/*
		 * Check if there is any action to be done for :
		 * - USB Reset received {XUSB_STATUS_RESET_MASK}
		 * - USB Suspend received {XUSB_STATUS_SUSPEND_MASK}
		 * - USB Disconnect received {XUSB_STATUS_DISCONNECT_MASK}
		 * - USB SOF received {XUSB_STATUS_SOF_PACKET_MASK}
		 */
		if (UsbInstPtr->HandlerFunc) {
			UsbInstPtr->HandlerFunc(UsbInstPtr->HandlerRef,
							IntrStatus);
		}
	}

	/*
	 * Check the buffer completion interrupts .
	 */
	if (IntrStatus & XUSB_STATUS_INTR_BUFF_COMP_ALL_MASK) {

		if (IntrStatus & XUSB_STATUS_EP0_BUFF1_COMP_MASK) {

			Ep = &UsbInstPtr->DeviceConfig.Ep[XUSB_EP_NUMBER_ZERO];
			if (Ep->HandlerFunc) {
				Ep->HandlerFunc(Ep->HandlerRef,
				XUSB_EP_NUMBER_ZERO, IntrStatus);

			}
		}
		BufIntr = ((IntrStatus & XUSB_STATUS_EP1_BUFF1_COMP_MASK) ||
			(IntrStatus & XUSB_STATUS_EP1_BUFF2_COMP_MASK));
		if (BufIntr) {

			Ep = &UsbInstPtr->DeviceConfig.Ep[XUSB_EP_NUMBER_ONE];
			if (Ep->HandlerFunc) {
				Ep->HandlerFunc(Ep->HandlerRef,
				XUSB_EP_NUMBER_ONE, IntrStatus);

			}
		}
		BufIntr = ((IntrStatus & XUSB_STATUS_EP2_BUFF1_COMP_MASK) ||
			(IntrStatus & XUSB_STATUS_EP2_BUFF2_COMP_MASK));
		if (BufIntr) {

			Ep = &UsbInstPtr->DeviceConfig.Ep[XUSB_EP_NUMBER_TWO];
			if (Ep->HandlerFunc) {
				Ep->HandlerFunc(Ep->HandlerRef,
				XUSB_EP_NUMBER_TWO, IntrStatus);
			}

		}
	}
	spin_unlock(&(udc->lock));

	return IRQ_HANDLED;
 }



 int usb_gadget_register_driver (struct usb_gadget_driver *driver)
 {
         struct xusb_udc *udc = &controller;
         int             retval;
	 const struct usb_endpoint_descriptor	*d = &config_bulk_out_desc;

         if (!driver
                         || driver->speed != USB_SPEED_HIGH
                         || !driver->bind
                         || !driver->unbind
                         || !driver->setup) {
                 DBG("bad parameter.\n");
                 return -EINVAL;
         }

         if (udc->driver) {
                 DBG("UDC already has a gadget driver\n");
                 return -EBUSY;
         }
         udc->driver = driver;
         udc->gadget.dev.driver = &driver->driver;

	 retval = device_add(&udc->gadget.dev);

         retval = driver->bind(&udc->gadget);
         if (retval) {
                 DBG("driver->bind() returned %d\n", retval);
                 udc->driver = NULL;
		 udc->gadget.dev.driver = NULL;
                 return retval;
         }

 	XUsb_Start(&udc->usb);
	udc->enabled = 1;

	xusb_ep_enable(&udc->ep[0].ep, d);
         return 0;
 }
EXPORT_SYMBOL (usb_gadget_register_driver);

 int usb_gadget_unregister_driver (struct usb_gadget_driver *driver)
 {
         struct xusb_udc *udc = &controller;
 	 unsigned long flags;

         if (!driver || driver != udc->driver)
                 return -EINVAL;

         spin_lock_irqsave(&udc->lock, flags);
  	 stop_activity(udc);
	 udc->enabled = 0;
         spin_unlock_irqrestore(&udc->lock, flags);

         driver->unbind(&udc->gadget);
	 device_del(&udc->gadget.dev);	
         DBG("unbound from %s\n", driver->driver.name);
 	 XUsb_Stop(&udc->usb);
         return 0;
 }
 EXPORT_SYMBOL (usb_gadget_unregister_driver);

 /*-------------------------------------------------------------------------*/
 static int __init xudc_probe(struct device *_dev)
 {
        struct xusb_udc *udc = &controller;
	struct platform_device *pdev = to_platform_device(_dev);
        int             retval = 0;
	struct resource *irq_res, *regs_res;
        XUsb_Config	ConfigPtr;
	u32 phy_addr;
        void *v_addr;

	spin_lock_init(&(udc->lock));
	udc->dev = _dev;

	/* Map the control registers in */
	irq_res = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
	regs_res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!regs_res || (regs_res->end - regs_res->start + 1 < 8)) {
		retval = -EFAULT;
		goto fail1;
	}

	udc->remap_size = regs_res->end - regs_res->start + 1;
	if (!request_mem_region(regs_res->start, udc->remap_size,
				driver_name)) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,18) 
		VDBG(KERN_ERR "Couldn't lock memory region at 0x%08X\n",
		       regs_res->start);
#else
		VDBG(KERN_ERR "Couldn't lock memory region at 0x%08lX\n",
		       regs_res->start);
#endif
		retval = -EBUSY;
		goto fail2;
	}

	v_addr = ioremap(regs_res->start, udc->remap_size);
	if (!v_addr) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,18) 
		VDBG(KERN_ERR "Couldn't ioremap memory at 0x%08X\n",
		       regs_res->start);
#else
		VDBG(KERN_ERR "Couldn't ioremap memory at 0x%08lX\n",
		       regs_res->start);
#endif
		retval = -EFAULT;
		goto fail3;
	}

	device_initialize(&udc->gadget.dev);

	ConfigPtr.DeviceId = pdev->id;
	ConfigPtr.BaseAddress = (u32)v_addr;
	phy_addr = regs_res->start;
        XUsb_CfgInitialize(&udc->usb, &ConfigPtr, (u32)v_addr);

        udc_reinit(udc);

	//MaxControlSize = 64;

	XUsb_SetDeviceAddress(&udc->usb, 0);

	/*
	 * Setup the interrupt handlers.
	 */
	XUsb_IntrSetHandler(&udc->usb, (void *)UsbIfIntrHandler,
				udc);

	XUsb_EpSetHandler(&udc->usb, XUSB_EP_NUMBER_ZERO,
			(XUsb_EpHandlerFunc *)Ep0IntrHandler, udc);

	XUsb_EpSetHandler(&udc->usb, XUSB_EP_NUMBER_ONE,
			(XUsb_EpHandlerFunc *)EpIntrHandler, udc);

	XUsb_EpSetHandler(&udc->usb, XUSB_EP_NUMBER_TWO,
			(XUsb_EpHandlerFunc *)EpIntrHandler, udc);

	XUsb_EpSetHandler(&udc->usb, XUSB_EP_NUMBER_THREE,
			(XUsb_EpHandlerFunc *)EpIntrHandler, udc);

	XUsb_EpSetHandler(&udc->usb, XUSB_EP_NUMBER_FOUR,
			(XUsb_EpHandlerFunc *)EpIntrHandler, udc);

	XUsb_EpSetHandler(&udc->usb, XUSB_EP_NUMBER_FIVE,
			(XUsb_EpHandlerFunc *)EpIntrHandler, udc);

	XUsb_EpSetHandler(&udc->usb, XUSB_EP_NUMBER_SIX,
			(XUsb_EpHandlerFunc *)EpIntrHandler, udc);

	XUsb_EpSetHandler(&udc->usb, XUSB_EP_NUMBER_SEVEN,
			(XUsb_EpHandlerFunc *)EpIntrHandler, udc);

        udc->enabled = 0;
		

        /* request UDC irqs */
        if (request_irq(irq_res->start, xusb_udc_irq, SA_RESTART, driver_name, udc)) {
              //   DBG("request irq %x failed\n", irq_res->start);
                 retval = -EBUSY;
                 goto fail1;
         }

	/*
	 * Enable the interrupts.
	 */
	XUsb_IntrEnable(&(udc->usb), (XUSB_STATUS_GLOBAL_INTR_MASK |
				XUSB_STATUS_RESET_MASK |
				XUSB_STATUS_DISCONNECT_MASK |
				XUSB_STATUS_SUSPEND_MASK |
				XUSB_STATUS_FIFO_BUFF_RDY_MASK |
				XUSB_STATUS_FIFO_BUFF_FREE_MASK |
				XUSB_STATUS_EP0_BUFF1_COMP_MASK |
				XUSB_STATUS_EP1_BUFF1_COMP_MASK |
				XUSB_STATUS_EP2_BUFF1_COMP_MASK |
				XUSB_STATUS_EP1_BUFF2_COMP_MASK |
				XUSB_STATUS_EP2_BUFF2_COMP_MASK));


        dev_set_drvdata(_dev, udc);
        create_debug_file(udc);
        retval = device_register(&udc->gadget.dev);
        if (retval < 0)
                goto fail0;


        INFO("%s version %s\n", driver_name, DRIVER_VERSION);
	VDBG(KERN_INFO "%s #%d at 0x%08X mapped to 0x%08X\n",
	       driver_name, udc->usb.Config.DeviceId,
	       phy_addr, (u32)v_addr);
        return 0;
fail3:
	iounmap((void *)(ConfigPtr.BaseAddress));

fail2:
	release_mem_region(regs_res->start, udc->remap_size);

fail1:
         device_unregister(&udc->gadget.dev);
	 stop_activity(udc);
fail0:
         DBG("%s probe failed, %d\n", driver_name, retval);
         return retval;
}

void UsbIfIntrHandler(void *CallBackRef, u32 IntrStatus)
{
	XUsb *InstancePtr;
	struct xusb_udc *udc;
	udc = (struct xusb_udc *)CallBackRef;
	InstancePtr = &(udc->usb);

	if (IntrStatus & XUSB_STATUS_RESET_MASK){
		VDBG("Reset \n");
		if(IntrStatus & XUSB_STATUS_HIGH_SPEED_MASK){
			udc->gadget.speed = USB_SPEED_HIGH;
			
		}else {
			udc->gadget.speed = USB_SPEED_FULL;
		}
		if (InstancePtr->DeviceConfig.CurrentConfiguration == 1) {
			InstancePtr->DeviceConfig.CurrentConfiguration = 0;
			XUsb_SetDeviceAddress(InstancePtr, 0);
		}

		XUsb_IntrDisable(InstancePtr, XUSB_STATUS_RESET_MASK);
		XUsb_IntrEnable(InstancePtr, (XUSB_STATUS_SUSPEND_MASK |
						XUSB_STATUS_DISCONNECT_MASK));
	}

	if (IntrStatus & XUSB_STATUS_DISCONNECT_MASK){
		XUsb_IntrDisable(InstancePtr,
				XUSB_STATUS_DISCONNECT_MASK);
		VDBG("Disconnect \n");
		if (InstancePtr->DeviceConfig.CurrentConfiguration == 1) {
			InstancePtr->DeviceConfig.CurrentConfiguration = 0;
			XUsb_SetDeviceAddress(InstancePtr, 0);
			XUsb_Start(&udc->usb);	
		}
		XUsb_IntrEnable(InstancePtr, (XUSB_STATUS_RESET_MASK |
					XUSB_STATUS_SUSPEND_MASK));
		stop_activity(udc);
	}

	if (IntrStatus & XUSB_STATUS_SUSPEND_MASK){
		VDBG("Suspend \n");
		XUsb_IntrDisable(InstancePtr, XUSB_STATUS_SUSPEND_MASK);
		XUsb_IntrEnable(InstancePtr, (XUSB_STATUS_RESET_MASK | 			
			XUSB_STATUS_DISCONNECT_MASK));
	}


}

void Ep0IntrHandler(void *CallBackRef, u8 EpNum, u32 IntrStatus)
{

	XUsb *InstancePtr;
	struct xusb_udc *udc;
	struct usb_ctrlrequest *ctrl;
	UsbCtrlRequest CtrlRqst;
	int Status;
	int epNum; 
	udc = (struct xusb_udc *)CallBackRef;
	InstancePtr = &(udc->usb);

	/*
	 * Process the end point zero buffer interrupt.
	 */
	if (IntrStatus & XUSB_STATUS_EP0_BUFF1_COMP_MASK){
		if (IntrStatus & XUSB_STATUS_SETUP_PACKET_MASK) {
			/*
			 * Received a setup packet. Execute the chapter 9
			 * command.
			 */
			XUsb_IntrEnable(InstancePtr,
				(XUSB_STATUS_DISCONNECT_MASK |
				XUSB_STATUS_SUSPEND_MASK |
				XUSB_STATUS_RESET_MASK));
			Status = Chapter9(InstancePtr, &CtrlRqst);
			if (Status != XST_SUCCESS){

				ctrl = (struct usb_ctrlrequest *) &CtrlRqst;
				/*
				 * Request is to be handled by the gadget
				 * driver.
				 */
				spin_unlock(&udc->lock);
		   		udc->driver->setup(&udc->gadget, ctrl);
				spin_lock(&udc->lock);
			} else {
				if (CtrlRqst.bRequest == CLEAR_FEATURE){
					epNum = CtrlRqst.wIndex & 0xf;
					udc->ep[epNum].stopped = 0;
				}				
				if (CtrlRqst.bRequest == SET_FEATURE){
					epNum = CtrlRqst.wIndex & 0xf;
					udc->ep[epNum].stopped = 1;
				}				
			}
		} else if (IntrStatus & XUSB_STATUS_FIFO_BUFF_RDY_MASK) {
				EP0ProcessOutToken(InstancePtr);
		} else if (IntrStatus & XUSB_STATUS_FIFO_BUFF_FREE_MASK) {
				EP0ProcessInToken(InstancePtr);
		}
	}
}

void EpIntrHandler(void *CallBackRef, u8 EpNum, u32 IntrStatus)
{

	XUsb *InstancePtr;
        struct xusb_request     *req;
	struct xusb_udc *udc;
	struct xusb_ep *ep;
	udc = (struct xusb_udc *)CallBackRef;
	InstancePtr = &(udc->usb);
	ep = &udc->ep[EpNum];
	/*
	 * Process the End point interrupts.
	 */
	if (IntrStatus & (USB_EP_BUFF1_MASK << EpNum)) {
		InstancePtr->DeviceConfig.Ep[EpNum].Buffer0Ready = 0;
	}

	if (IntrStatus & (USB_EP_BUFF2_MASK << EpNum)) {
		InstancePtr->DeviceConfig.Ep[EpNum].Buffer1Ready = 0;
	}

        if (list_empty(&ep->queue)){
		req = NULL;
	}
	else {
		req = list_entry(ep->queue.next,
				struct xusb_request, queue); 
	}
	if (!req){
		return;
	} 
	if (ep->is_in)	{	
		write_fifo(ep, req);
	}else {
	        read_fifo(ep, req);
	}
}

static void xudc_shutdown(struct device *_dev)
{
	VDBG("Shut Down\n");
	/*
 	 * Do nothing.
	 */
}
 static int __devexit xudc_remove(struct device *_dev)
 {
        
	struct xusb_udc *udc = dev_get_drvdata(_dev);
	struct resource *irq_res;
	struct platform_device *pdev = to_platform_device(_dev);
  
	/* Map the control registers in */
	irq_res = platform_get_resource(pdev, IORESOURCE_IRQ, 0);

        DBG("remove\n");

        if (udc->driver != 0)
                 usb_gadget_unregister_driver(udc->driver);

        free_irq(irq_res->start, udc);

	dev_set_drvdata(_dev, NULL);

        return 0;
 }
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,18) 
static int xudc_resume(struct device *dev)
#else
static int xudc_resume(struct device *dev, u32 level)
#endif
 {

	VDBG("resume called\n");
         /* maybe reconnect to host; if so, clocks on */
         return 0;
 }

static struct device_driver udc_driver = {
	 .name 		 = "xlnx_udc",
	 .bus 		 = &platform_bus_type,
         .probe          = xudc_probe,
         .remove         = xudc_remove,
         .shutdown       = xudc_shutdown,
         .resume         = xudc_resume,
 };

static int __init udc_init(void)
{
	int retvalue;

	retvalue = driver_register(&udc_driver);
	if (!retvalue) {
		VDBG(KERN_INFO "Registered Usb device %s: version %s\n", 
			driver_name, DRIVER_VERSION);
	}else {
		VDBG("USB driver_register failed \n");
	}
	
	return retvalue;
}
module_init(udc_init);

static void __devexit udc_exit(void)
{
	driver_unregister(&udc_driver);
}
module_exit(udc_exit);

MODULE_DESCRIPTION("XILINX udc driver");
MODULE_AUTHOR("Vidhumouli, Hunsigida");
MODULE_LICENSE("GPL");



