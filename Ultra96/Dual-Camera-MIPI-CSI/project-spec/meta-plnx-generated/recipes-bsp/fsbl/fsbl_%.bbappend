
YAML_SERIAL_CONSOLE_STDIN_forcevariable = "psu_uart_1"
XSCTH_WS = "${TOPDIR}/../components/plnx_workspace/fsbl"
EXTERNALXSCTSRC = "${PETALINUX}/tools/hsm/data/embeddedsw"
EXTERNALXSCTSRCHASH = "build"
inherit externalxsctsrc
YAML_SERIAL_CONSOLE_STDOUT_forcevariable = "psu_uart_1"
EXTERNALXSCTSRC_BUILD = "${TOPDIR}/../components/plnx_workspace/fsbl"
export _JAVA_OPTIONS
_JAVA_OPTIONS = "-Duser.home=${TMPDIR}/xsctenv"
