#
# Copyright (c) 2000-2009 Андрей Валяев <dron@infosec.ru>
# This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
#

GRUB2_SVN=http://svn.savannah.gnu.org/svn/grub/trunk/grub2
# 2316 - появилось меню, но как буд-то не дочитал grub.cfg, только первый пункт
GRUB2_REV=2440

GRUB2_ROOT=${CURDIR}/grub2

GRUB2_MODULES=normal sh ls cat help multiboot elf configfile

MDF_ROOT=${CURDIR}/MDF.Root
MDF_PACKAGES=Stub-IA32 Hello KernelInfo SchedulerTest TPCTest
release : MDF_PACKAGES_CLEANUP=MDF-libc Kernel-IA32 KernelLib Core Stub-IA32-Types

DATE=${shell date +%Y%m%d}

all help :
	@echo "Варианты сборки: iso - grub legasy iso, iso2 - grub2 iso, flash - grub2 flash."

.PHONY: iso mdf-current-legacy.iso
iso mdf-current-legacy.iso : root
	rm -rf ${MDF_TEMP}/iso mdf-current-legacy.iso
	mkdir -p ${MDF_TEMP}/iso/MDF.Root
	mkdir -p ${MDF_TEMP}/iso/MDF
	mkdir -p ${MDF_TEMP}/iso/MDF.iso

	cp -a iso/boot ${MDF_TEMP}/iso/boot

	# бинарники
	cp -a ${MDF_ROOT}/* ${MDF_TEMP}/iso/MDF.Root/

	# Исходники
	cp -a ${MDF_REPOS}/* ${MDF_TEMP}/iso/MDF/

	# Исходники для сборки имиджей
	cp -a qemu ${MDF_TEMP}/iso/MDF.iso/
	cp -a .bochsrc ${MDF_TEMP}/iso/MDF.iso/
	cp -a Makefile ${MDF_TEMP}/iso/MDF.iso/
	cp -a iso ${MDF_TEMP}/iso/MDF.iso/

	# Зачистка от SVN информации
	find ${MDF_TEMP}/iso -name .svn | xargs rm -rf

	mkisofs -b boot/grub/stage2_eltorito -no-emul-boot -boot-load-size 4 \
		-boot-info-table -iso-level 3 -r -J -input-charset=utf8 \
		-publisher "Dron <dron@infosec.ru>" \
		-o mdf-current-legacy.iso ${MDF_TEMP}/iso

.PHONY: iso2 mdf-current.iso
iso2 mdf-current.iso : root grub2
	rm -rf ${MDF_TEMP}/iso2 ${MDF_TEMP}/iso2.tmp mdf-current.iso
	mkdir -p ${MDF_TEMP}/iso2/MDF.Root
	mkdir -p ${MDF_TEMP}/iso2/MDF
	mkdir -p ${MDF_TEMP}/iso2/MDF.iso
	mkdir -p ${MDF_TEMP}/iso2.tmp

	# TODO Создать образ GRUB
	mkdir -p ${MDF_TEMP}/iso2/boot/grub
	${GRUB2_ROOT}/bin/grub-mkimage -o ${MDF_TEMP}/iso2.tmp/grub-image.tmp \
		${GRUB2_MODULES} biosdisk iso9660
	cp ${GRUB2_ROOT}/lib/grub/i386-pc/cdboot.img ${MDF_TEMP}/iso2/boot/grub/grub-image
	dd if=${MDF_TEMP}/iso2.tmp/grub-image.tmp of=${MDF_TEMP}/iso2/boot/grub/grub-image seek=1
	cp grub.cfg ${MDF_TEMP}/iso2/boot/grub/

	# бинарники
	cp -a ${MDF_ROOT}/* ${MDF_TEMP}/iso2/MDF.Root/

	# Исходники
	cp -a ${MDF_REPOS}/* ${MDF_TEMP}/iso2/MDF/

	# Исходники для сборки имиджей
	cp -a qemu ${MDF_TEMP}/iso2/MDF.iso/
	cp -a .bochsrc ${MDF_TEMP}/iso2/MDF.iso/
	cp -a Makefile ${MDF_TEMP}/iso2/MDF.iso/

	# Зачистка от SVN информации
	find ${MDF_TEMP}/iso2 -name .svn | xargs rm -rf

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

.PHONY: release-legacy
release-legacy mdf-${DATE}-legacy.iso.bz2 : mdf-current-legacy.iso
	cp $< mdf-${DATE}-legacy.iso
	bzip2 -f mdf-${DATE}-legacy.iso

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

.PHONY: grub2 grubclean
grub2 : ${GRUB2_ROOT}/grub-r${GRUB2_REV}

${GRUB2_ROOT}/grub-r${GRUB2_REV} :
	rm -rf ${MDF_TEMP}/grub2 ${GRUB2_ROOT}
	svn export ${GRUB2_SVN}@${GRUB2_REV} ${MDF_TEMP}/grub2
	patch -d ${MDF_TEMP}/grub2 -p0 < ${CURDIR}/mb_elf_sym.patch
	cd ${MDF_TEMP}/grub2; ./autogen.sh
	cd ${MDF_TEMP}/grub2; ./configure --prefix=${GRUB2_ROOT} \
		--disable-largefile
	make -C ${MDF_TEMP}/grub2 all install
	touch ${GRUB2_ROOT}/grub-r${GRUB2_REV}

grubclean:
	rm -rf grub2
