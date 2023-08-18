#!/vendor/bin/sh

MSIM_DEVICE=0

if grep -q "Model: F5122" /dev/block/bootdevice/by-name/LTALabel; then
    MSIM_DEVICE=1
fi

if [[ "${MSIM_DEVICE}" -eq 1 ]]; then
    setprop persist.vendor.radio.multisim.config dsds
else
    setprop persist.vendor.radio.block_allow_data 1
    setprop persist.vendor.radio.multisim.config ss
fi
