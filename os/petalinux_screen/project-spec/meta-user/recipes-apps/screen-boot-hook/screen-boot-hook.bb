#
# This file is the screen-boot-hook recipe.
#

SUMMARY = "Launch /opt/screen/scripts/run_service.sh on boot"
SECTION = "PETALINUX/apps"
LICENSE = "MIT"

LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI += "file://screen-boot-hook.sh \
            file://screen-boot-hook.service"

inherit systemd

SYSTEMD_SERVICE:${PN} = "screen-boot-hook.service"
SYSTEMD_AUTO_ENABLE:${PN} = "enable"

do_install() {
    install -d ${D}${bindir}
    install -m 0755 ${WORKDIR}/screen-boot-hook.sh ${D}${bindir}/screen-boot-hook.sh

    install -d ${D}${systemd_system_unitdir}
    install -m 0644 ${WORKDIR}/screen-boot-hook.service \
        ${D}${systemd_system_unitdir}/screen-boot-hook.service
}
