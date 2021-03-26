%ifndef SECOND_STAGE

jmp short bpb_end
nop

%endif

pbp_OEM:                resb 8  ; +00
bpb_BytesPerSector:     resb 2  ; +08
bpb_SectorsPerCluster:  resb 1  ; +10
bpb_ReservedSectors:    resb 2  ; +11
bpb_NumberOfFATs:       resb 1  ; +13
bpb_RootEntries:        resb 2  ; +14
bpb_TotalSectors:       resb 2  ; +16
bpb_Media:              resb 1  ; +18
bpb_SectorsPerFAT:      resb 2  ; +19
bpb_SectorsPerTrack:    resb 2  ; +21
bpb_HeadsPerCylinder:   resb 2  ; +23
bpb_HiddenSectors:      resb 4  ; +25
bpb_TotalSectorsBig:    resb 4  ; +29
; Extended
bpb_DriveNumber:        resb 1  ; +33
bpb_Unused:             resb 1  ; +34
bpb_ExtBootSignature:   resb 1  ; +35
bpb_SerialNumber:       resb 4  ; +36
bpb_VolumeLabel:        resb 11 ; +40
bpb_FileSystem:         resb 8  ; +51

%ifndef SECOND_STAGE

times 0x5A-($-$$) db 0          ; Pad for compatibility
bpb_end:
nop

%endif