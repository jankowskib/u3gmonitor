USB MONITOR FOR 3G DONGLES
==========================

This is my implementation of u3gmonitor, originally developed by ALLWINNER TECH (https://github.com/allwinner-ics/device_softwinner_common/tree/master/packages/u3gmonitor)

<dl>
  <dt>What it does?</dt>
  <dd>
 * It watches for connected USB devices
   </dd>
      <dd>
 * Automatically calls usb_modeswitch
    </dd>
   <dd>
 * Contains some workaround for dongles, which aren't recognized by usb option driver
  </dd>
</dl>
<dl>
  <dt>Installation guide</dt>
  <dd>
 * Add service to init.rc:
  </dd>  
</dl>
	service u3gmonitor /system/bin/u3gmonitor
	class main
	user root
	group root
	oneshot
