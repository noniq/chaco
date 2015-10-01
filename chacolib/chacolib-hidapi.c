
hid_device *ch_hid;

/******************************************************************************/

static void printerror(const char *func, int rc) 
{
#ifdef DEBUG
    LOGERR("%s: %p (%d)\n", func, ch_hid, rc);
#else
    if (rc != 0) {
        LOGERR("%s: %d\n", func, rc);
    }
#endif
}

int chameleon_usb_init(void)
{
    ch_hid = NULL;
    int rc;

    DBG("chameleon_usb_init\n");

    ch_hid = hid_open(CHAMELEON_VID, CHAMELEON_PID, NULL);
    if(ch_hid == NULL) {
        LOGERR("Can't initialize hid!\n");
        return -1;
    }

    //hid_set_nonblocking(ch_hid, 1);
    
    return 0;
}

int chameleon_usb_close(void)
{
    int rc;
    DBG("chameleon_usb_close\n");

    hid_close(ch_hid);

    ch_hid= NULL;
    return 0;
}

/* read data from chameleon */
int chameleon_readdata(USBHIDDataFrame *buf, int timeout)
{
    int length = 0;
    
    //int rc = hid_read_timeout( ch_hid, (unsigned char *)buf, SIZEOF_USBHIDDATAFRAME, timeout);
    int rc = hid_read( ch_hid, (unsigned char *)buf, SIZEOF_USBHIDDATAFRAME);
    
    if(rc == SIZEOF_USBHIDDATAFRAME) {
      return 0;
    }
    printerror("chameleon_readdata", rc);
    return -1;
}

/* write data to chameleon */
int chameleon_writedata(USBHIDDataFrame * Data)
{
    int length = 0;
 
    /*parameter 2: Endpoint address. Should be read out of descriptor table*/
    int rc = hid_write(ch_hid, (const unsigned char*)Data, SIZEOF_USBHIDDATAFRAME);

    if(rc == SIZEOF_USBHIDDATAFRAME) {
      return 0;
    }
    printerror("chameleon_writedata", rc);
    return -1;
}


/* check if chameleon is connected and initalised */
int chameleon_checkconfig(void)
{
    int rc;
    int dummy = 0;

    if (ch_hid) {
#if 0
        rc = libusb_get_configuration(ch_hnd, &dummy);
        if (rc < 0) {
            return -1;
        }
        rc = chameleon_getversion(NULL, NULL);
        if (rc < 0) {
            return -1;
        }
        rc = chameleon_getstatus(NULL, NULL, NULL, NULL, NULL);
        if (rc < 0) {
            return -1;
        }
#endif
        return CHACO_OK;
    }
    return -1;
}

int chameleon_find(void)
{
#if 0
    int rc;
    struct libusb_device_descriptor desc;

    if(ch_hnd)
    {
        return CHACO_OK;
    }

    LOGVER("searching for Chameleon on USB bus\n");

    ch_hnd = libusb_open_device_with_vid_pid (lusb_ctx, CHAMELEON_VID, CHAMELEON_PID);
    if(ch_hnd)
    {
        LOGVER("Found Chameleon on USB bus\n");
    }
    else
    {
        ch_hnd = NULL;
        return -1;
    }

    ch_dev = libusb_get_device (ch_hnd);
    if(ch_dev) {
        LOGVER("Got Chameleon device\n");
    }
    else
    {
        LOGERR("Can't get Chameleon device\n");
    ch_hnd = NULL;
        return -1;
    }
    rc = libusb_get_device_descriptor(ch_dev, &desc);
    if (rc < 0) {
        LOGERR("failed to get device descriptor\n");
        ch_hnd = NULL;
        return -1;
    }
#endif
    return CHACO_OK;
}

int chameleon_claim(void)
{
    int rc;

#if 0
    rc = libusb_kernel_driver_active(ch_hnd, 0);
    if (rc > 0) {
        rc = libusb_detach_kernel_driver(ch_hnd, 0);
    }

    rc = libusb_claim_interface(ch_hnd, 0);
    if (rc < 0) {
        return -1;
    }
#endif

    return CHACO_OK;
}
