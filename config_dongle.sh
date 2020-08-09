#!/bin/bash

print_exec() {
	echo -e "\t$1"
	eval $1
}

config_iface() {
	print_exec "ip link set $1 down"
	print_exec "iw $1 set type monitor"
	print_exec "iw $1 set monitor fcsfail"
}

if [[ "$1" == "-h" || "$1" == "--help" ]]; then
	echo -e "$0\
		\n\t-k		Kill all processes listed by airmon-zc\
		\n\t-d [phyX]	Disable Carrier Sense for physical device phyX\
		\n\t-i [iface]	Configure only interface [iface]\
		\n\t-h		Show this help"
	exit
fi

while getopts "d:k:i:h" option; do
	case ${option} in
	    d) PHY=${OPTARG}
		    NOCS=true
		    ;;
	    k) KILL=true;;
	    i) ONLY_ONE_IFACE=true
		    IFACE=${OPTARG}
		    ;;

	esac
done

if [[ $NOCS ]]; then
	echo -e "\nDisable Carrier Sense"
    print_exec "echo 1 > /sys/kernel/debug/ieee80211/$PHY/ath9k_htc/registers/force_channel_idle"
    print_exec "echo 1 > /sys/kernel/debug/ieee80211/$PHY/ath9k_htc/registers/ignore_virt_cs"
    exit
fi

if [[ $KILL ]]; then
	echo airmon-zc check
	ret=$(airmon-zc check | grep -E -o "^[0-9]{3,4}")
	if [[ $ret != "" ]]; then
		echo "$ret" | while IFS= read -r line; do
			print_exec "kill $line"
		done
	else
		echo "No process to kill."
	fi
fi

echo -e "\nConfigure intrefaces"

ifnames=$(iw dev | grep -E -o "Interface (.+)" | grep -E -o "\s.+$" | grep -E -o "[a-z0-9]+")
echo "$ifnames" | while IFS= read -r interface; do
	if [[ ! $ONLY_ONE_IFACE ]]; then
		config_iface $interface
	elif [[ $IFACE == $interface ]]; then
		config_iface $interface
		break
	fi
done

echo -e "\nEnable full debug output for dmesg"
phynum=$(iw dev | grep -E -o "^phy#[0-9]+$" | grep -E -o "[0-9]+$")
echo "$phynum" | while IFS= read -r num; do
	print_exec "echo 0xffffffff > /sys/kernel/debug/ieee80211/phy$num/ath9k_htc/debug"
done

echo -e "\n\nCheck config"
if [[ $ONLY_ONE_IFACE ]]; then
	iw $IFACE info
else
	iw dev
fi
