#
# Copyright (c) 2000-2009 Андрей Валяев <dron@infosec.ru>
# This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
#

GRUB2_VERSION=1.98
GRUB2_ARCHIVE=grub-${GRUB2_VERSION}.tar.gz

GRUB2_ROOT=${CURDIR}/grub2

GRUB2_MODULES=normal sh ls cat help multiboot elf configfile

MDF_ROOT=${CURDIR}/MDF.Root
MDF_PACKAGES=Stub-IA32 Hello KernelInfo SchedulerTest TPCTest Console
release : MDF_PACKAGES_CLEANUP=MDF-libc Kernel-IA32 KernelLib Core Stub-IA32-Types

DATE=${shell date +%Y%m%d}

all help :
	@echo "Варианты сборки: iso2 - grub2 iso, flash - grub2 flash."

.PHONY: iso2 mdf-current.iso
iso2 mdf-current.iso : root grub2
	rm -rf ${MDF_TEMP}/iso2 ${MDF_TEMP}/iso2.tmp mdf-current.iso
	mkdir -p ${MDF_TEMP}/iso2

	# Создать образ GRUB
	mkdir -p ${MDF_TEMP}/iso2/boot/grub
	mkdir -p ${MDF_TEMP}/iso2.tmp
	${GRUB2_ROOT}/bin/grub-mkimage -o ${MDF_TEMP}/iso2.tmp/grub-image.tmp \
		${GRUB2_MODULES} biosdisk iso9660
	cp ${GRUB2_ROOT}/lib/grub/i386-pc/cdboot.img ${MDF_TEMP}/iso2/boot/grub/grub-image
	dd if=${MDF_TEMP}/iso2.tmp/grub-image.tmp of=${MDF_TEMP}/iso2/boot/grub/grub-image seek=1
	cp grub.cfg ${MDF_TEMP}/iso2/boot/grub/

	# бинарники
	cp -a ${MDF_ROOT} ${MDF_TEMP}/iso2/MDF.Root

	# Исходники
	cp -a ${MDF_REPOS} ${MDF_TEMP}/iso2/MDF
	rm -rf ${MDF_TEMP}/iso2/MDF/Images/* ${MDF_TEMP}/iso2/MDF/.hg*

	# Исходники для сборки имиджей
	cp -a Makefile qemu .bochsrc ${MDF_TEMP}/iso2/MDF/Images/
	cp -a grub.cfg ${MDF_TEMP}/iso2/MDF/Images/
	#cp -a qemu.disk ${MDF_TEMP}/iso2/MDF/Images/

	mkisofs -b boot/grub/grub-image -no-emul-boot -boot-load-size 4 \
		-boot-info-table -iso-level 3 -r -J -input-charset=utf8 \
		-publisher "Dron <dron@infosec.ru>" \
		-o mdf-current.iso ${MDF_TEMP}/iso2

flash mdf-current.flash : root
	rm -rf ${MDF_TEMP}/flash2 ${MDF_TEMP}/flash2.tmp mdf-current.flash
	mkdir -p ${MDF_TEMP}/flash2/MDF.Root
	mkdir -p ${MDF_TEMP}/flash2.tmp

	cp -a ${MDF_ROOT}/* ${MDF_TEMP}/flash2/MDF.Root/
	
	mkdir -p ${MDF_TEMP}/flash2/boot/grub
	cp grub.cfg ${MDF_TEMP}/flash2/boot/grub/

	${GRUB2_ROOT}/bin/grub-mkrescue --image-type=floppy \
		--overlay=${MDF_TEMP}/flash2 \
		"--modules=${GRUB2_MODULES} memdisk tar" \
		mdf-current.flash

	#tar -cpf ${MDF_TEMP}/flash2.tmp/rootdisk.tar -C ${MDF_TEMP}/flash2/ \
	#	boot MDF.Root

	#${GRUB2_ROOT}/bin/grub-mkimage -o ${MDF_TEMP}/flash2.tmp/grub-image.tmp \
	#	${GRUB2_MODULES} memdisk tar \
	#	-m ${MDF_TEMP}/flash2.tmp/rootdisk.tar

	#cp ${GRUB2_ROOT}/lib/grub/i386-pc/boot.img mdf-current.flash
	#dd if=${MDF_TEMP}/flash2.tmp/grub-image.tmp of=mdf-current.flash seek=1

.PHONY: release
release : mdf-${DATE}.iso.bz2 mdf-${DATE}.flash

mdf-${DATE}.iso.bz2 : mdf-current.iso
	cp $< mdf-${DATE}.iso
	bzip2 -f mdf-${DATE}.iso

# Флешку бесполезно архивировать, она и так lzma пожата.
mdf-${DATE}.flash : mdf-current.flash
	cp $< mdf-${DATE}.flash

.PHONY: root rootclean
root:
	msetup MDF-Env
	msetup ${MDF_PACKAGES}
	mremove ${MDF_PACKAGES_CLEANUP}

rootclean:
	rm -rf MDF.Root

.PHONY: grub2 grub2sig grubclean 
grub2 : ${GRUB2_ROOT}/grub-${GRUB2_VERSION}

${MDF_FILES}/${GRUB2_ARCHIVE} :
	wget ftp://alpha.gnu.org/gnu/grub/${GRUB2_ARCHIVE} -O $@

${MDF_FILES}/${GRUB2_ARCHIVE}.sig:
	wget ftp://alpha.gnu.org/gnu/grub/${GRUB2_ARCHIVE}.sig -O $@

${MDF_FILES}/grub-newreloc.patch.bz2:
	wget http://mdf.googlecode.com/files/grub-newreloc.patch.bz2 -O $@

grub2sig: ${MDF_FILES}/${GRUB2_ARCHIVE} ${MDF_FILES}/${GRUB2_ARCHIVE}.sig
	gpg --keyserver keys.gnupg.net --recv-keys E82E4209
	gpg --verify ${MDF_FILES}/${GRUB2_ARCHIVE}.sig

${GRUB2_ROOT}/grub-${GRUB2_VERSION} : ${MDF_FILES}/${GRUB2_ARCHIVE} \
		${MDF_FILES}/grub-newreloc.patch.bz2
	rm -rf ${MDF_TEMP}/grub-${GRUB2_VERSION} ${GRUB2_ROOT}
	tar -xz -C ${MDF_TEMP} -f ${MDF_FILES}/${GRUB2_ARCHIVE} 
	bzcat ${MDF_FILES}/grub-newreloc.patch.bz2 | \
		patch -d ${MDF_TEMP}/grub-${GRUB2_VERSION} -p1
	cd ${MDF_TEMP}/grub-${GRUB2_VERSION}; ./configure \
		--prefix=${GRUB2_ROOT} --disable-largefile --disable-werror
	make -C ${MDF_TEMP}/grub-${GRUB2_VERSION} all install
	touch $@

grubclean:
	rm -rf grub2
