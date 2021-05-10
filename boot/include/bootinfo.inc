%ifndef __BOOTINFO_INC__
%define __BOOTINFO_INC__

struc bootinfo
    .memoryMapAddress   resd 1
    .memoryMapEntries   resd 1
    .memorySizeLow      resd 1
    .memorySizeHigh     resd 1
    .bootDevice         resd 1
    .screen_width       resd 1
    .screen_height      resd 1
    .screen_pitch       resd 1
    .framebuffer        resd 1
endstruc

%endif ; __BOOTINFO_INC__