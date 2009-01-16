/* pro_types.h */


int _dwarf_transform_simplename_to_disk (
    Dwarf_P_Debug       dbg,
    enum dwarf_sn_kind        entrykind,
    int                 section_index, /* in de_elf_sects etc */
    Dwarf_Error         *error
);

