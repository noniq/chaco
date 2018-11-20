
static struct libusb_device * ch_dev = NULL;
static libusb_device_handle * ch_hnd = NULL;
static libusb_context * lusb_ctx = NULL;

/******************************************************************************/

static const char *geterrstr(int rc)
{ 
    switch(rc)
    {
        case LIBUSB_SUCCESS:
            return "OK";
        case LIBUSB_ERROR_IO:
            return "LIBUSB_ERROR_IO - Input/output error";
        case LIBUSB_ERROR_NOT_FOUND:
            return "LIBUSB_ERROR_NOT_FOUND - Entity not found";
        case LIBUSB_ERROR_BUSY:
            return "LIBUSB_ERROR_BUSY - Resource busy";
        case LIBUSB_ERROR_INTERRUPTED:
            return "LIBUSB_ERROR_INTERRUPTED - System call interrupted (perhaps due to signal)";
        case LIBUSB_ERROR_NO_MEM:
            return "LIBUSB_ERROR_NO_MEM - Insufficient memory";
        case LIBUSB_ERROR_OTHER:
            return "LIBUSB_ERROR_OTHER";
        case LIBUSB_ERROR_TIMEOUT:
            return "LIBUSB_ERROR_TIMEOUT - Operation timed out";
        case LIBUSB_ERROR_PIPE:
            return "LIBUSB_ERROR_PIPE - Pipe error";
        case LIBUSB_ERROR_OVERFLOW:
            return "LIBUSB_ERROR_OVERFLOW";
        case LIBUSB_ERROR_NO_DEVICE:
            return "LIBUSB_ERROR_NO_DEVICE - No such device (it may have been disconnected)";
        case LIBUSB_ERROR_ACCESS:
            return "LIBUSB_ERROR_ACCESS - Access denied (insufficient permissions)";
        case LIBUSB_ERROR_INVALID_PARAM:
            return "LIBUSB_ERROR_INVALID_PARAM - Invalid parameter";
        case LIBUSB_ERROR_NOT_SUPPORTED:
            return "LIBUSB_ERROR_NOT_SUPPORTED - Operation not supported or unimplemented on this platform";
        default:
            return "Unknown";
    }
}

static void printerror(const char *func, int rc) 
{
#ifdef DEBUG
    LOGERR("%s: ctx:%p dev:%p hdl:%p (%d) %s\n", func,lusb_ctx, ch_dev, ch_hnd, rc, geterrstr(rc));
#else
    if (rc != 0) {
        LOGERR("%s: %s\n", func, geterrstr(rc));
    }
#endif
}

static int chameleon_usb_init(void)
{
    //libusb initializing
    ch_dev = NULL;
    ch_hnd = NULL;
    int rc;

    DBG("chameleon_usb_init\n");
    if (lusb_ctx) {
        LOGVER("libusb already initialized.\n");
        return 0;
    }

    rc = libusb_init(&lusb_ctx);
    if(rc != 0)
    {
        LOGERR("Can't initialize libusb\n");
        return -1;
    }
#ifdef DEBUG
        libusb_set_debug(lusb_ctx, 3);
#endif
    return 0;
}

static int chameleon_usb_close(void)
{
    int rc;
    DBG("chameleon_usb_close\n");
    if (ch_hnd) {
        rc = libusb_release_interface(ch_hnd,0);
        if (rc < 0) {
            printerror("chameleon_usb_close", rc);
        }
        libusb_close(ch_hnd);
        ch_hnd = NULL;
    }
    if (lusb_ctx) {
        libusb_exit(lusb_ctx);
    }
    lusb_ctx = NULL;
    return 0;
}

/* read data from chameleon */
static int chameleon_readdata(USBHIDDataFrame *buf, int timeout)
{
    int length = 0;
    int rc = libusb_interrupt_transfer(ch_hnd,129, (unsigned char*)buf,34 ,&length,timeout);
    
    if(rc == 0)return 0;
    printerror("chameleon_readdata", rc);
    
    return -1;
}

/* write data to chameleon */
static int chameleon_writedata(USBHIDDataFrame * Data)
{
    int length = 0;
 
    if(Data == NULL) return -1;

    /*parameter 2: Endpoint address. Should be read out of descriptor table*/
    int rc = libusb_interrupt_transfer(ch_hnd,2, (unsigned char*)Data,34 ,&length,1000);

    /* it seems that in windows length isnt always 34 !? */
#ifdef LINUX
    if((rc == 0) && (length == 34))return 0;
    printerror("chameleon_writedata", rc);
#else
    if(rc == 0) return 0;
    printerror("chameleon_writedata", rc);
#endif
    return -1;
}


/* check if chameleon is connected and initalised */
int chameleon_checkconfig(void)
{
    int rc;
    int dummy = 0;

    if (ch_hnd) {
        rc = libusb_get_configuration(ch_hnd, &dummy);
        if (rc < 0) {
            return -1;
        }
        rc = chameleon_getversion(NULL, NULL);
        if (rc < 0) {
            return -1;
        }
#if 0
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

    return CHACO_OK;
}

int chameleon_claim(void)
{
    int rc;

    rc = libusb_kernel_driver_active(ch_hnd, 0);
    if (rc > 0) {
        rc = libusb_detach_kernel_driver(ch_hnd, 0);
    }

    rc = libusb_claim_interface(ch_hnd, 0);
    if (rc < 0) {
        return -1;
    }

    return CHACO_OK;
}
