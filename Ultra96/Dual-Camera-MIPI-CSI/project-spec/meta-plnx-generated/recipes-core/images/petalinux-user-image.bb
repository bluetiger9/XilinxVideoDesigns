DESCRIPTION = "PETALINUX image definition for Xilinx boards"
LICENSE = "MIT"

require recipes-core/images/petalinux-image-common.inc 

inherit extrausers
IMAGE_LINGUAS = " "

IMAGE_INSTALL = "\
		kernel-modules \
		mtd-utils \
		canutils \
		openssh-sftp-server \
		git \
		pciutils \
		run-postinsts \
		udev-extraconf \
		openamp-fw-echo-testd \
		openamp-fw-mat-muld \
		openamp-fw-rpc-demo \
		packagegroup-core-boot \
		packagegroup-core-ssh-dropbear \
		tcf-agent \
		bridge-utils \
		hellopm \
		packagegroup-petalinux \
		packagegroup-petalinux-matchbox \
		packagegroup-petalinux-openamp \
		packagegroup-petalinux-self-hosted \
		packagegroup-petalinux-v4lutils \
		packagegroup-petalinux-x11 \
		libftdi \
		cmake \
		iperf3 \
		lmsensors-sensorsdetect \
		packagegroup-base-extended \
		packagegroup-petalinux-96boards-sensors \
		packagegroup-petalinux-ultra96-webapp \
		python-pyserial \
		python3-pip \
		ultra96-ap-setup \
		"
EXTRA_USERS_PARAMS = "usermod -P root root;"
