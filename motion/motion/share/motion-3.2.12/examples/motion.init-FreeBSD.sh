#!/bin/sh
#
# motion.sh for rc.d usage 2006 Angel Carpintero 
#
# Add the following line in /etc/rc.conf to enable motion at startup
#
# motion_enable="YES"
#

# PROVIDE: motion 
# REQUIRE: DAEMON
# KEYWORD: shutdown

. /etc/rc.subr

motion_enable="${motion_enable-NO}"

name="motion"
rcvar=`set_rcvar`

command="/opt/project1/motion/motion-3.2.12/_install/bin/${name}"
pidfile="/var/run/${name}.pid"
required_files="/usr/local/etc/${name}.conf"

load_rc_config $name
run_rc_command "$1"
