/*
   It's impossible for callers of dwarf_srclines() to get to and
   free all the resources (in particular, the li_context and its
   lc_file_entries). 
   So this function, new July 2005, does it.  
*/

int
dwarf_srclines_dealloc(Dwarf_Dbg dbg, Dwarf_Line linebuf,
		       Dwarf_Signed count, Dwarf_Error * error)
{

    struct Dwarf_Line_Context_s *context = 0;

    if (count > 0) {
	/* All these entries share a single context */
	context = linebuf[0]->li_context;
    }
    for (i = 0; i < count; ++i) {
	dwarf_dealloc(dbg, linebuf[i], DW_DLA_LINE);
    }
    dwarf_dealloc(dbg, linebuf, DW_DLA_LIST);

    if (context) {
	Dwarf_File_Entry fe = context->lc_file_entries;
	int curct = 0;

	while (fe) {
	    Dwarf_File_Entry fenext = fe->fi_next;

	    dwarf_dealloc(dbg, fe, DW_DLA_FILE_ENTRY)
		fe = fenext;
	}
	dwarf_dealloc(dbg, context, DW_DLA_LINE_CONTEXT);
    }

    return DW_DLV_OK;
}
