SRC_URI += "file://bsp.cfg \
            file://user_2018-10-17-20-29-00.cfg \
            file://user_2018-10-18-21-31-00.cfg \
            file://user_2018-10-18-21-45-00.cfg \
            file://user_2018-10-18-22-33-00.cfg \
            file://0001-v4l2-fh-call-v4l2_pipeline_pm_use.patch \
            file://0001-ov5647.c-latest-changes-power-on-debug.patch \
            file://0001-ov5647.c-power-oen-debug.patch \
            file://0001-ov5647.c-power-oen-debug-00-pin-1-disable.patch \
            file://0001-ov5647.c-power-oen-debug-port-0-1-y-disable.patch \
            file://0001-ov5647.c-power-oen-debug-port-0-1-high-y-disable.patch \
            file://0001-ov5647.c-power-new-reg-oen-values.patch \
            file://0001-ov5647.c-fix-regs-mipi-rx-dbg-stream-on-off.patch \
            file://0001-ov5647.c-revert-drive-strength.patch \
            file://0001-xilinx_frmbuf.c-debug-info.patch \
            file://user_2018-11-24-20-13-00.cfg \
            file://0001-xilinx_frmbuf.c-debug-info-fix.patch \
            file://0001-ov5647.c-add-1920x1080-and-1920x1440-modes.patch \
            "

FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"
