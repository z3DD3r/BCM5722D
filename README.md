BCM5722D
========

Unofficial Mac OS X driver for Broadcom's BCM5722 NetXtreme and NetLink
family of gigabit Ethernet controllers. It is implemented based on the
BCM5722 Programmer's Guide provided in Broadcom's open source developer
resource. Additional information is gleaned from Linux(tg3) and
FreeBSD(if\_bge) driver.

It supports the following models:

* BCM5722
* BCM5754
* BCM5754M
* BCM5752
* BCM5755
* BCM5755M
* BCM5761
* BCM5761E
* BCM57780
* BCM57781
* BCM57785
* BCM57788
* BCM5784M
* BCM5787
* BCM5787M
* BCM5906
* BCM5906M

Installation
------------

Install to `/System/Library/Extensions`

Method of installation

* Manually copy the kext to /S/L/E and repair permissions, or
* Use [KextWizard](http://www.insanelymac.com/forum/index.php?showtopic=253395).

or

Install in kext dir of Clover

License
-------

This project is released under the GNU General Public License Version 2. Please
see LICENSE file or <http://www.gnu.org/licenses/gpl-2.0.html> for detailed
license information.

Credits
-------

* Early beta testers at the InsanelyMac forum (acero, Hacktrix2006, quadomatic,
  queshaolangman, Zprood)
* tg3(Linux) and bge(FreeeBSD) driver authors

Disclaimer
----------

This driver is neither supported nor endorsed by Broadcom. Use at your own risk.
