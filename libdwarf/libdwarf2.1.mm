\."
\." the following line may be removed if the ff ligature works on your machine
.lg 0
\." set up heading formats
.ds HF 3 3 3 3 3 2 2
.ds HP +2 +2 +1 +0 +0
.nr Hs 5
.nr Hb 5
\." ==============================================
\." Put current date in the following at each rev
.ds vE rev 1.44, 23 Aug 2000
\." ==============================================
\." ==============================================
.ds | |
.ds ~ ~
.ds ' '
.if t .ds Cw \&\f(CW
.if n .ds Cw \fB
.de Cf          \" Place every other arg in Cw font, beginning with first
.if \\n(.$=1 \&\*(Cw\\$1\fP
.if \\n(.$=2 \&\*(Cw\\$1\fP\\$2
.if \\n(.$=3 \&\*(Cw\\$1\fP\\$2\*(Cw\\$3\fP
.if \\n(.$=4 \&\*(Cw\\$1\fP\\$2\*(Cw\\$3\fP\\$4
.if \\n(.$=5 \&\*(Cw\\$1\fP\\$2\*(Cw\\$3\fP\\$4\*(Cw\\$5\fP
.if \\n(.$=6 \&\*(Cw\\$1\fP\\$2\*(Cw\\$3\fP\\$4\*(Cw\\$5\fP\\$6
.if \\n(.$=7 \&\*(Cw\\$1\fP\\$2\*(Cw\\$3\fP\\$4\*(Cw\\$5\fP\\$6\*(Cw\\$7\fP
.if \\n(.$=8 \&\*(Cw\\$1\fP\\$2\*(Cw\\$3\fP\\$4\*(Cw\\$5\fP\\$6\*(Cw\\$7\fP\\$8
.if \\n(.$=9 \&\*(Cw\\$1\fP\\$2\*(Cw\\$3\fP\\$4\*(Cw\\$5\fP\\$6\*(Cw\\$7\fP\\$8\
*(Cw
..
.nr Cl 4
.SA 1
.TL
A Consumer Library Interface to DWARF 
.AF ""
.AU "UNIX International Programming Languages Special Interest Group" 
.PF "'\*(vE'- \\\\nP -''"
.PM ""
.AS 1
This document describes an interface to a library of functions
.FS 
UNIX is a registered trademark of UNIX System Laboratories, Inc.
in the United States and other countries.
.FE
to access DWARF debugging information entries and DWARF line number
information. It does not make recommendations as to how the functions
described in this document should be implemented nor does it
suggest possible optimizations. 
.P
The document is oriented to reading DWARF version 2.
There are certain sections which are SGI-specific (those
are clearly identified in the document).
We would propose this 
to the PLSIG committee
as the basis for a
standard libdwarf interface, but as of this
writing, the committee is dormant.
.P
This document are subject to change.
.P
\*(vE

.AE
.MT 4

.H 1 "INTRODUCTION"
This document describes an interface to \fIlibdwarf\fP, a
library of functions to provide access to DWARF debugging information
records, DWARF line number information, DWARF address range and global 
names information, weak names information, DWARF frame description 
information, DWARF static function names, DWARF static variables, and 
DWARF type information.
.P

.H 2 "Purpose and Scope"
The purpose of this document is to document a library of functions 
to access DWARF debugging information. There is no effort made in 
this document to address the creation of these records as those
issues are addressed separately.

.P
Additionally, the focus of this document is the functional interface,
and as such, implementation as well as optimization issues are
intentionally ignored.

.H 2 "Definitions"
DWARF debugging information entries (DIE) are the segments of information 
placed in the \f(CW.debug_*\fP sections by compilers, assemblers, and 
linkage editors that, in conjunction with line number entries, are 
necessary for symbolic source-level debugging.  Refer to the document 
"\fIDWARF Debugging Information Format\fP" from UI PLSIG for a more 
complete description of these entries.

.P
This document adopts all the terms and definitions in "\fIDWARF Debugging 
Information Format\fP" version 2.  It focuses on the implementation at
Silicon Graphics Computer Systems.  Although we believe the interface
is general enough to be of interest to other vendors too, there are a
few places where changes may need to be made.

.H 2 "Overview"
The remaining sections of this document describe the proposed interface
to \f(CWlibdwarf\fP, first by describing the purpose of additional types
defined by the interface, followed by descriptions of the available 
operations.  This document assumes you are thoroughly familiar with the 
information contained in the \fIDWARF Debugging Information Format\fP 
document. 
.P
We separate the functions into several categories to emphasize that not 
all consumers want to use all the functions.  We call the categories 
Debugger, Internal-level, High-level, and Miscellaneous not because one is more 
important than another but as a way of making the rather large set of 
function calls easier to understand.
.P
Unless otherwise specified, all functions and structures should be
taken as being designed for Debugger consumers.
.P
The Debugger Interface of this library is intended to be used by debuggers. 
The interface is low-level (close to dwarf) but suppresses irrelevant detail.
A debugger will want to absorb all of some sections at startup and will 
want to see little or nothing of some sections except at need.  And even 
then will probably want to absorb only the information in a single compilation 
unit at a time.  A debugger does not care about
implementation details of the library.
.P
The Internal-level Interface is for a DWARF prettyprinter and checker.  
A 
thorough prettyprinter will want to know all kinds of internal things 
(like actual FORM numbers and actual offsets) so it can check for 
appropriate structure in the DWARF data and print (on request) all 
that internal information for human users and libdwarf authors and 
compiler-writers.  
Calls in this interface provide data a debugger 
does not care about.
.P
The High-level Interface is for higher level access
(it's not really a high level interface!).  
Programs such as 
disassemblers will want to be able to display relevant information 
about functions and line numbers without having to invest too much 
effort in looking at DWARF.
.P
The miscellaneous interface is just what is left over: the error handler 
functions.
.P
The following is a brief mention of the changes in this libdwarf from 
the libdwarf draft for DWARF Version 1.
.H 2 "Items Changed"
dwarf_nextglob(), dwarf_globname(), and dwarf_globdie() were all changed 
to operate on the items in the .debug_pubnames section.
.P
All functions were modified to return solely an error code.
Data is returned through pointer arguments.
This makes writing safe and correct library-using-code far easier.
For justification for this approach, see the book by
Steve Maguire titled "Writing Solid Code" at your bookstore.


.H 2 "Items Removed"
.P
Dwarf_Type
was removed since types are no longer special.
.P
dwarf_typeof()
was removed since types are no longer special.
.P
Dwarf_Ellist
was removed since element lists no longer are a special format.
.P
Dwarf_Bounds
was removed since bounds have been generalized.
.P
dwarf_nextdie()
was replaced by dwarf_next_cu_header() to reflect the
real way dwarf is organized.
The dwarf_nextdie() was only useful for getting to compilation
unit beginnings, so it does not seem harmful to remove it in favor
of a more direct function.
.P
dwarf_childcnt() is removed on grounds
that no good use was apparent.
.P
dwarf_prevline() and dwarf_nextline() were removed on grounds this
is better left to a debugger to do.
Similarly, dwarf_dieline() was removed.
.P
dwarf_is1stline() was removed as it was not meaningful for the
revised dwarf line operations.
.P
Any libdwarf implementation might well decide to support all the
removed functionality and to retain the DWARF Version 1 meanings
of that functionality.  
This would be difficult because the
original libdwarf draft
specification used traditional C library interfaces which
confuse the values returned by successful calls with
exceptional conditions like failures and 'no more data' indications.

.H 2 "Revision History"
.VL 15
.LI "March 93"
Work on dwarf2 SGI draft begins
.LI "June 94"
The function returns are changed to return an error/success code
only.
.LE

.H 1 "Types Definitions"

.H 2 "General Description"
The \fIlibdwarf.h\fP header file contains typedefs and preprocessor 
definitions of types and symbolic names used to reference objects 
of \fIlibdwarf\fP. The types defined by typedefs contained in 
\fIlibdwarf.h\fP all use the convention of adding \f(CWDwarf_\fP 
as a prefix and can be placed in three categories: 

.BL
.LI
Scalar types : The scalar types defined in \fIlibdwarf.h\fP are
defined primarily for notational convenience and identification.
Depending on the individual definition, they are interpreted as a 
value, a pointer, or as a flag.
.LI
Aggregate types : Some values can not be represented by a single 
scalar type; they must be represented by a collection of, or as a 
union of, scalar and/or aggregate types. 
.LI
Opaque types : The complete definition of these types is intentionally
omitted; their use is as handles for query operations, which will yield
either an instance of another opaque type to be used in another query, or 
an instance of a scalar or aggregate type, which is the actual result.
.P

.H 2 "Scalar Types"
The following are the defined by \fIlibdwarf.h\fP:

.DS
\f(CW
typedef int                Dwarf_Bool;
typedef unsigned long long Dwarf_Off;
typedef unsigned long long Dwarf_Unsigned;
typedef unsigned short     Dwarf_Half;
typedef unsigned char      Dwarf_Small;
typedef signed long long   Dwarf_Signed;
typedef unsigned long long Dwarf_Addr;
typedef void 		   *Dwarf_Ptr;
typedef void   (*Dwarf_Handler)(Dwarf_Error *error, Dwarf_Ptr errarg);
.DE

.nr aX \n(Fg+1
Dwarf_Ptr is an address for use by the host program calling the library,
not for representing pc-values/addresses within the target object file.
Dwarf_Addr is for pc-values within the target object file.  The sample 
scalar type assignments above are for a \fIlibdwarf.h\fP that can read 
and write
32-bit or 64-bit binaries on a 32-bit or 64-bit host machine.
The types must be  defined appropriately
for each implementation of libdwarf.
A description of these scalar types in the SGI/MIPS
environment is given in Figure \n(aX.

.DS
.TS
center box, tab(:);
lfB lfB lfB lfB
l c c l.
NAME:SIZE:ALIGNMENT:PURPOSE
_
Dwarf_Bool:4:4:Boolean states
Dwarf_Off:8:8:Unsigned file offset
Dwarf_Unsigned:8:8:Unsigned large integer
Dwarf_Half:2:2:Unsigned medium integer
Dwarf_Small:1:1:Unsigned small integer
Dwarf_Signed:8:8:Signed large integer
Dwarf_Addr:8:8:Program address
:::(target program)
Dwarf_Ptr:4|8:4|8:Dwarf section pointer 
:::(host program)
Dwarf_Handler:4|8:4|8:Pointer to
:::error handler function 
.TE
.FG "Scalar Types"
.DE

.H 2 "Aggregate Types"
The following aggregate types are defined by 
the SGI 
\fIlibdwarf.h\fP:
\f(CWDwarf_Loc\fP,
\f(CWDwarf_Locdesc\fP,
\f(CWDwarf_Block\fP, 
\f(CWDwarf_Frame_Op\fP. 
While most of \f(CWlibdwarf\fP acts on or returns simple values or
opaque pointer types, this small set of structures seems useful.

.H 3 "Location Record"
The \f(CWDwarf_Loc\fP type identifies a single atom of a location description
or a location expression.

.DS
\f(CWtypedef struct {
        Dwarf_Small        lr_atom;
        Dwarf_Unsigned     lr_number;
        Dwarf_Unsigned     lr_number2;
        Dwarf_Unsigned     lr_offset;  
} Dwarf_Loc;\fP
.DE

The \f(CWlr_atom\fP identifies the atom corresponding to the \f(CWDW_OP_*\fP 
definition in \fIdwarf.h\fP and it represents the operation to be performed 
in order to locate the item in question.

.P
The \f(CWlr_number\fP field is the operand to be used in the calculation
specified by the \f(CWlr_atom\fP field; not all atoms use this field.
Some atom operations imply signed numbers so it is necessary to cast 
this to a \f(CWDwarf_Signed\fP type for those operations.

.P
The \f(CWlr_number2\fP field is the second operand specified by the 
\f(CWlr_atom\fP field; only \f(CWDW_OP_BREGX\fP has this field.  Some 
atom operations imply signed numbers so it may be necessary to cast 
this to a \f(CWDwarf_Signed\fP type for those operations.

.P
The \f(CWlr_offset\fP field is the byte offset (within the block the 
location record came from) of the atom specified by the \f(CWlr_atom\fP 
field.  This is set on all atoms.  This is useful for operations 
\f(CWDW_OP_SKIP\fP and \f(CWDW_OP_BRA\fP.

.H 3 "Location Description"
The \f(CWDwarf_Locdesc\fP type represents an ordered list of 
\f(CWDwarf_Loc\fP records used in the calculation to locate 
an item.  Note that in many cases, the location can only be 
calculated at runtime of the associated program.

.DS
\f(CWtypedef struct {
        Dwarf_Addr        ld_lopc;
        Dwarf_Addr        ld_hipc;
        Dwarf_Unsigned    ld_cents;
        Dwarf_Loc*        ld_s;
} Dwarf_Locdesc;\fP
.DE

The \f(CWld_lopc\fP and \f(CWld_hipc\fP fields provide an address range for
which this location descriptor is valid.  Both of these fields are set to
\fIzero\fP if the location descriptor is valid throughout the scope of the
item it is associated with.  These addresses are virtual memory addresses, 
not offsets-from-something.  The virtual memory addresses do not account 
for dso movement (none of the pc values from libdwarf do that, it is up to 
the consumer to do that).

.P
The \f(CWld_cents\fP field contains a count of the number of \f(CWDwarf_Loc\fP 
entries pointed to by the \f(CWld_s\fP field.

.P
The \f(CWld_s\fP field points to an array of \f(CWDwarf_Loc\fP records. 

.H 3 "Data Block"
.SP
The \f(CWDwarf_Block\fP type is used to contain the value of an attribute
whose form is either \f(CWDW_FORM_block1\fP, \f(CWDW_FORM_block2\fP, 
\f(CWDW_FORM_block4\fP, \f(CWDW_FORM_block8\fP, or \f(CWDW_FORM_block\fP.
Its intended use is to deliver the value for an attribute of any of these 
forms.

.DS
\f(CWtypedef struct {
        Dwarf_Unsigned     bl_len;
        Dwarf_Ptr          bl_data;
} Dwarf_Block;\fP
.DE

.P
The \f(CWbl_len\fP field contains the length in bytes of the data pointed
to by the \f(CWbl_data\fP field. 

.P
The \f(CWbl_data\fP field contains a pointer to the uninterpreted data.
Since we use  a \f(CWDwarf_Ptr\fP here one must copy the pointer to some 
other type (typically an \f(CWunsigned char *\fP) so one can add increments 
to index through the data.  The data pointed to by \f(CWbl_data\fP is not 
necessarily at any useful alignment.

.H 3 "Frame Operation Codes"
The \f(CWDwarf_Frame_Op\fP type is used to contain the data of a single
instruction of an instruction-sequence of low-level information from the 
section containing frame information.  This is ordinarily used by 
Internal-level 
Consumers trying to print everything in detail.

.DS
\f(CWtypedef struct {
	Dwarf_Small  fp_base_op;
	Dwarf_Small  fp_extended_op;
	Dwarf_Half   fp_register;
	Dwarf_Signed fp_offset;
	Dwarf_Offset fp_instr_offset;
} Dwarf_Frame_Op;
.DE

\f(CWfp_base_op\fP is the 2-bit basic op code.  \f(CWfp_extended_op\fP is 
the 6-bit extended opcode (if \f(CWfp_base_op\fP indicated there was an 
extended op code) and is zero otherwise.
.P
\f(CWfp_register\fP 
is any (or the first) register value as defined
in the \f(CWCall Frame Instruction Encodings\fP figure
in the \f(CWdwarf\fP document.
If not used with the Op it is 0.
.P
\f(CWfp_offset\fP
is the address, delta, offset, or second register as defined
in the \f(CWCall Frame Instruction Encodings\fP figure
in the \f(CWdwarf\fP document.
If this is an \f(CWaddress\fP then the value should be cast to
\f(CW(Dwarf_Addr)\fP before being used.
In any implementation this field *must* be as large as the
larger of Dwarf_Signed and Dwarf_Addr for this to work properly.
If not used with the op it is 0.
.P
\f(CWfp_instr_offset\fP is the byte_offset (within the instruction
stream of the frame instructions) of this operation.  It starts at 0
for a given frame descriptor.
.H 3 "Macro Details Record"
The \f(CWDwarf_Macro_Details\fP type gives information about
a single entry in the .debug.macinfo section.
.DS
\f(CWstruct Dwarf_Macro_Details_s {
  Dwarf_Off    dmd_offset;
  Dwarf_Small  dmd_type;  
  Dwarf_Signed dmd_lineno;
  Dwarf_Signed dmd_fileindex;
  char *       dmd_macro;
};
typedef struct Dwarf_Macro_Details_s Dwarf_Macro_Details;
.DE
.P
\f(CWdmd_offset\fP is the byte offset, within the .debug_macinfo
section, of this macro information.
.P
\f(CWdmd_type\fP is the type code of this macro info entry
(or 0, the type code indicating that this is the end of
macro information entries for a compilation unit.
See \f(CWDW_MACINFO_define\fP, etc in the DWARF document.
.P
\f(CWdmd_lineno\fP is the line number where this entry was found,
or 0 if there is no applicable line number.
.P
\f(CWdmd_fileindex\fP is the file index of the file involved.
This is only guaranteed meaningful on a \f(CWDW_MACINFO_start_file\fP
\f(CWdmd_type\fP.  Set to -1 if unknown (see the functional
interface for more details).
.P
\f(CWdmd_macro\fP is the applicable string.
For a \f(CWDW_MACINFO_define\fP
this is the macro name and value.
For a
\f(CWDW_MACINFO_undef\fP, or
this is the macro name.
For a
\f(CWDW_MACINFO_vendor_ext\fP
this is the vendor-defined string value.
For other \f(CWdmd_type\fPs this is 0.

.H 2 "Opaque Types"
The opaque types declared in \fIlibdwarf.h\fP are used as descriptors
for queries against dwarf information stored in various debugging 
sections.  Each time an instance of an opaque type is returned as a 
result of a \fIlibdwarf\fP operation (\f(CWDwarf_Debug\fP excepted), 
it should be free'd, using \f(CWdwarf_dealloc()\fP when it is no longer 
of use.  
Some functions return a number of instances of an opaque type 
in a block, by means of a pointer to the block and a count of the number
of opaque descriptors in the block:
see the function description for deallocation rules for such functions.
The list of opaque types defined 
in \fIlibdwarf.h\fP that are pertinent to the Consumer Library, and their 
intended use is described below.

.DS
\f(CWtypedef struct Dwarf_Debug_s* Dwarf_Debug;\fP
.DE
An instance of the \f(CWDwarf_Debug\fP type is created as a result of a 
successful call to \f(CWdwarf_init()\fP, or \f(CWdwarf_elf_init()\fP, 
and is used as a descriptor for subsequent access to most \f(CWlibdwarf\fP
functions on that object.  The storage pointed to by this descriptor 
should be not be free'd, using the \f(CWdwarf_dealloc()\fP function.
Instead free it with \f(CWdwarf_finish()\fP.
.P

.DS
\f(CWtypedef struct Dwarf_Die_s* Dwarf_Die;\fP
.DE
An instance of a \f(CWDwarf_Die\fP type is returned from a successful 
call to the \f(CWdwarf_siblingof()\fP, \f(CWdwarf_child\fP, or 
\f(CWdwarf_offdie()\fP function, and is used as a descriptor for queries 
about information related to that DIE.  The storage pointed to by this 
descriptor should be free'd, using \f(CWdwarf_dealloc()\fP with the allocation 
type \f(CWDW_DLA_DIE\fP when no longer needed.

.DS
\f(CWtypedef struct Dwarf_Line_s* Dwarf_Line;\fP
.DE
Instances of \f(CWDwarf_Line\fP type are returned from a successful call 
to the \f(CWdwarf_srclines()\fP function, and are used as descriptors for 
queries about source lines.  The storage pointed to by these descriptors
should be individually free'd, using \f(CWdwarf_dealloc()\fP with the 
allocation type \f(CWDW_DLA_LINE\fP when no longer needed.

.DS
\f(CWtypedef struct Dwarf_Global_s* Dwarf_Global;\fP
.DE
Instances of \f(CWDwarf_Global\fP type are returned from a successful 
call to the \f(CWdwarf_get_globals()\fP function, and are used as 
descriptors for queries about global names (pubnames).  The storage 
pointed to by these descriptors should be individually free'd, using 
\f(CWdwarf_dealloc()\fP with the allocation type \f(CWDW_DLA_GLOBAL\fP, 
when no longer needed.

.DS
\f(CWtypedef struct Dwarf_Weak_s* Dwarf_Weak;\fP
.DE
Instances of \f(CWDwarf_Weak\fP type are returned from a successful call 
to the 
SGI-specific \f(CWdwarf_get_weaks()\fP
function, and are used as descriptors for 
queries about weak names.  The storage pointed to by these descriptors 
should be individually free'd, using \f(CWdwarf_dealloc()\fP with the 
allocation type \f(CWDW_DLA_WEAK\fP when no longer needed.

.DS
\f(CWtypedef struct Dwarf_Func_s* Dwarf_Func;\fP
.DE
Instances of \f(CWDwarf_Func\fP type are returned from a successful
call to the 
SGI-specific \f(CWdwarf_get_funcs()\fP
function, and are used as 
descriptors for queries about static function names.  
The storage 
pointed to by these descriptors should be individually free'd, using 
\f(CWdwarf_dealloc()\fP with the allocation type \f(CWDW_DLA_FUNC\fP, 
when no longer needed.

.DS
\f(CWtypedef struct Dwarf_Type_s* Dwarf_Type;\fP
.DE
Instances of \f(CWDwarf_Type\fP type are returned from a successful call 
to the 
SGI-specific \f(CWdwarf_get_types()\fP
function, and are used as descriptors for 
queries about user defined types.  
The storage pointed to by this descriptor 
should be individually free'd, using \f(CWdwarf_dealloc()\fP with the 
allocation type \f(CWDW_DLA_TYPENAME\fP when no longer needed.

.DS
\f(CWtypedef struct Dwarf_Var_s* Dwarf_Var;\fP
.DE
Instances of \f(CWDwarf_Var\fP type are returned from a successful call 
to the SGI-specific \f(CWdwarf_get_vars()\fP
function, and are used as descriptors for 
queries about static variables.  
The storage pointed to by this descriptor 
should be individually free'd, using \f(CWdwarf_dealloc()\fP with the 
allocation type \f(CWDW_DLA_VAR\fP when no longer needed.

.DS
\f(CWtypedef struct Dwarf_Error_s* Dwarf_Error;\fP
.DE
This descriptor points to a structure that provides detailed information
about errors detected by \f(CWlibdwarf\fP.  Users typically provide a
location for \f(CWlibdwarf\fP to store this descriptor for the user to
obtain more information about the error.  The storage pointed to by this
descriptor should be free'd, using \f(CWdwarf_dealloc()\fP with the 
allocation type \f(CWDW_DLA_ERROR\fP when no longer needed.

.DS
\f(CWtypedef struct Dwarf_Attribute_s* Dwarf_Attribute;\fP
.DE
Instances of \f(CWDwarf_Attribute\fP type are returned from a successful 
call to the \f(CWdwarf_attrlist()\fP, or \f(CWdwarf_attr()\fP functions, 
and are used as descriptors for queries about attribute values.  The storage 
pointed to by this descriptor should be individually free'd, using 
\f(CWdwarf_dealloc()\fP with the allocation type \f(CWDW_DLA_ATTR\fP when 
no longer needed.

.DS
\f(CWtypedef struct Dwarf_Abbrev_s* Dwarf_Abbrev;\fP
.DE
An instance of a \f(CWDwarf_Abbrev\fP type is returned from a successful 
call to \f(CWdwarf_get_abbrev()\fP, and is used as a descriptor for queries 
about abbreviations in the .debug_abbrev section.  The storage pointed to 
by this descriptor should be free'd, using \f(CWdwarf_dealloc()\fP with the
allocation type \f(CWDW_DLA_ABBREV\fP when no longer needed.

.DS
\f(CWtypedef struct Dwarf_Fde_s* Dwarf_Fde;\fP
.DE
Instances of \f(CWDwarf_Fde\fP type are returned from a successful call 
to the \f(CWdwarf_get_fde_list()\fP, \f(CWdwarf_get_fde_for_die()\fP, or
\f(CWdwarf_get_fde_at_pc()\fP functions, and are used as descriptors for 
queries about frames descriptors.  The storage pointed to by these 
descriptors should be individually free'd, using \f(CWdwarf_dealloc()\fP 
with the allocation type \f(CWDW_DLA_FDE\fP when no longer needed.

.DS
\f(CWtypedef struct Dwarf_Cie_s* Dwarf_Cie;\fP
.DE
Instances of \f(CWDwarf_Cie\fP type are returned from a successful call 
to the \f(CWdwarf_get_fde_list()\fP function, and are used as descriptors 
for queries about information that is common to several frames.  The 
storage pointed to by this descriptor should be individually free'd, using 
\f(CWdwarf_dealloc()\fP with the allocation type \f(CWDW_DLA_CIE\fP when 
no longer needed.

.DS
\f(CWtypedef struct Dwarf_Arange_s* Dwarf_Arange;\fP
.DE
Instances of \f(CWDwarf_Arange\fP type are returned from successful calls 
to the \f(CWdwarf_get_aranges()\fP, or \f(CWdwarf_get_arange()\fP functions, 
and are used as descriptors for queries about address ranges.  The storage 
pointed to by this descriptor should be individually free'd, using 
\f(CWdwarf_dealloc()\fP with the allocation type \f(CWDW_DLA_ARANGE\fP when 
no longer needed.

.H 1 "Error Handling"
The method for detection and disposition of error conditions that arise 
during access of debugging information via \fIlibdwarf\fP is consistent
across all \fIlibdwarf\fP functions that are capable of producing an
error.  This section describes the method used by \fIlibdwarf\fP in
notifying client programs of error conditions. 

.P
Most functions within \fIlibdwarf\fP accept as an argument a pointer to 
a \f(CWDwarf_Error\fP descriptor where a \f(CWDwarf_Error\fP descriptor 
is stored if an error is detected by the function.  Routines in the client 
program that provide this argument can query the \f(CWDwarf_Error\fP 
descriptor to determine the nature of the error and perform appropriate 
processing. 

.P
A client program can also specify a function to be invoked upon detection 
of an error at the time the library is initialized (see \f(CWdwarf_init()\fP). 
When a \fIlibdwarf\fP routine detects an error, this function is called
with two arguments: a code indicating the nature of the error and a pointer
provided by the client at initialization (again see \f(CWdwarf_init()\fP).
This pointer argument can be used to relay information between the error 
handler and other routines of the client program.  A client program can 
specify or change both the error handling function and the pointer argument 
after initialization using \f(CWdwarf_seterrhand()\fP and 
\f(CWdwarf_seterrarg()\fP.

.P
In the case where \fIlibdwarf\fP functions are not provided a pointer
to a \f(CWDwarf_Error\fP descriptor, and no error handling function was 
provided at initialization, \fIlibdwarf\fP functions terminate execution 
by calling \f(CWabort(3C)\fP.

.P
The following lists the processing steps taken upon detection of an
error:
.AL 1
.LI
Check the \f(CWerror\fP argument; if not a \fINULL\fP pointer, allocate
and initialize a \f(CWDwarf_Error\fP descriptor with information describing
the error, place this descriptor in the area pointed to by \f(CWerror\fP,
and return a value indicating an error condition.
.LI
If an \f(CWerrhand\fP argument was provided to \f(CWdwarf_init()\fP
at initialization, call \f(CWerrhand()\fP passing it the error descriptor
and the value of the \f(CWerrarg\fP argument provided to \f(CWdwarf_init()\fP. 
If the error handling function returns, return a value indicating an 
error condition.
.LI
Terminate program execution by calling \f(CWabort(3C)\fP.
.LE
.SP

In all cases, it is clear from the value returned from a function 
that an error occurred in executing the function, since
DW_DLV_ERROR is returned.
.P
As can be seen from the above steps, the client program can provide
an error handler at initialization, and still provide an \f(CWerror\fP
argument to \fIlibdwarf\fP functions when it is not desired to have
the error handler invoked.

.P
If a \f(CWlibdwarf\fP function is called with invalid arguments, the 
behaviour is undefined.  In particular, supplying a \f(CWNULL\fP pointer 
to a \f(CWlibdwarf\fP function (except where explicitly permitted), 
or pointers to invalid addresses or uninitialized data causes undefined 
behaviour; the return value in such cases is undefined, and the function 
may fail to invoke the caller supplied error handler or to return a 
meaningful error number.  Implementations also may abort execution for 
such cases.

.P
.H 2 "Returned values in the functional interface"
Values returned by \f(CWlibdwarf\fP functions to indicate 
success and errors
.nr aX \n(Fg+1
are enumerated in Figure \n(aX.
The \f(CWDW_DLV_NO_ENTRY\fP
case is useful for functions 
need to indicate that while there was no data to return
there was no error either.
For example, \f(CWdwarf_siblingof()\fP
may return \f(CWDW_DLV_NO_ENTRY\fP to indicate that that there was
no sibling to return.
.DS
.TS
center box, tab(:);
lfB cfB lfB 
l c l.
SYMBOLIC NAME:VALUE:MEANING
_
DW_DLV_ERROR:1:Error
DW_DLV_OK:0:Successful call
DW_DLV_NO_ENTRY:-1:No applicable value
.TE
.FG "Error Indications"
.DE
.P
Each function in the interface that returns a value returns one
of the integers in the above figure.
.P
If \f(CWDW_DLV_ERROR\fP is returned and a pointer to a \f(CWDwarf_Error\fP
pointer is passed to the function, then a Dwarf_Error handle is returned
thru the pointer. No other pointer value in the interface returns a value.
.P
If \f(CWDW_DLV_NO_ENTRY\fP is returned no pointer value in the
interface returns a value.
.P
If \f(CWDW_DLV_NO_OK\fP is returned  the \f(CWDwarf_Error\fP pointer, if
supplied, is not touched, but any other values to be returned
through pointers are returned.
.P
Pointers passed to allow values to be returned thru them are 
uniformly the last pointers
in each argument list.
.P
All the interface functions are defined from the point of view of
the writer-of-the-library (as is traditional for UN*X library
documentation), not from the point of view of the user of the library.
The caller might code:
.P
.DS
Dwarf_Line line;
Dwarf_Signed ret_loff;
Dwarf_Error  err;
int retval = dwarf_lineoff(line,&ret_loff,&err); 
.DE
for the function defined as
.P
.DS
int dwarf_lineoff(Dwarf_Line line,Dwarf_Signed *return_lineoff,
  Dwarf_Error* err);
.DE
and this document refers to the function as 
returning the value thru *err or *return_lineoff or 
uses the phrase "returns in
the location pointed to by err".
Sometimes other similar phrases are used.

.H 1 "Memory Management"
Several of the functions that comprise \fIlibdwarf\fP return pointers 
(opaque descriptors) to structures that have been dynamically allocated 
by the library.  To aid in the management of dynamic memory, the function 
\f(CWdwarf_dealloc()\fP is provided to free storage allocated as a result 
of a call to a \fIlibdwarf\fP function.  This section describes the strategy 
that should be taken by a client program in managing dynamic storage.

.H 2 "Read-only Properties"
All pointers (opaque descriptors) returned by or as a result of a 
\fIlibdwarf Consumer Library\fP 
call should be assumed to point to read-only memory.  
The results are undefined for \fIlibdwarf\fP  clients that attempt 
to write to a region pointed to by a value returned by a 
\fIlibdwarf Consumer Library\fP 
call.

.H 2 "Storage Deallocation"
In some cases the pointers returned by a \fIlibdwarf\fP call are pointers
to data which is not free-able.  
The library knows from the allocation type
provided to it whether the space is freeable or not and will not free 
inappropriately when \f(CWdwarf_dealloc()\fP is called.  
So it is vital
that \f(CWdwarf_dealloc()\fP be called with the proper allocation type.
.P
For most storage allocated by \fIlibdwarf\fP, the client can free the
storage for reuse by calling \f(CWdwarf_dealloc()\fP, providing it with 
the \f(CWDwarf_Debug\fP descriptor specifying the object for which the
storage was allocated, a pointer to the area to be free-ed, and an 
identifier that specifies what the pointer points to (the allocation
type).  For example, to free a \f(CWDwarf_Die die\fP belonging the the
object represented by \f(CWDwarf_Debug dbg\fP, allocated by a call to 
\f(CWdwarf_siblingof()\fP, the call to \f(CWdwarf_dealloc()\fP would be:
.DS
    \f(CWdwarf_dealloc(dbg, die, DW_DLA_DIE);\fP
.DE

To free storage allocated in the form of a list of pointers (opaque 
descriptors), each member of the list should be deallocated, followed 
by deallocation of the actual list itself.  The following code fragment 
uses an invocation of \f(CWdwarf_attrlist()\fP as an example to illustrate 
a technique that can be used to free storage from any \fIlibdwarf\fP 
routine that returns a list:
.DS
\f(CWDwarf_Unsigned atcnt;
Dwarf_Attribute *atlist;
int errv;

if ((errv = dwarf_attrlist(somedie, &atlist,&atcnt, &error)) == DW_DLV_OK) {

        for (i = 0; i < atcnt; ++i) {
                /* use atlist[i] */
                dwarf_dealloc(dbg, atlist[i], DW_DLA_ATTR);
        }
        dwarf_dealloc(dbg, atlist, DW_DLA_LIST);
}\fP
.DE

The \f(CWDwarf_Debug\fP returned from \f(CWdwarf_init()\fP 
or \f(CWdwarf_elf_init()\fP 
cannot be free'd using \f(CWdwarf_dealloc()\fP.
The function \f(CWdwarf_finish()\fP will deallocate all dynamic storage
associated with an instance of a \f(CWDwarf_Debug\fP type.  In particular,
it will deallocate all dynamically allocated space associated with the
\f(CWDwarf_Debug\fP descriptor, and finally make the descriptor invalid.

An \f(CWDwarf_Error\fP returned from \f(CWdwarf_init()\fP
or \f(CWdwarf_elf_init()\fP
in case of a failure cannot be free'd
using \f(CWdwarf_dealloc()\fP.
The only way to free the \f(CWDwarf_Error\fP from either of those
calls is to use \f2free(3)\fP directly.
Every \f(CWDwarf_Error\fP must be free'd 
by \f(CWdwarf_dealloc()\fP except those
returned by \f(CWdwarf_init()\fP
or \f(CWdwarf_elf_init()\fP.

.P
The codes that identify the storage pointed to in calls to 
.nr aX \n(Fg+1
\f(CWdwarf_dealloc()\fP are described in figure \n(aX.
.DS
.TS
center box, tab(:);
lfB lfB 
l l.
IDENTIFIER:USED TO FREE 
_
DW_DLA_STRING           :     char* 
DW_DLA_LOC              :     Dwarf_Loc 
DW_DLA_LOCDESC          :     Dwarf_Locdesc 
DW_DLA_ELLIST           :     Dwarf_Ellist (not used)
DW_DLA_BOUNDS           :     Dwarf_Bounds (not used) 
DW_DLA_BLOCK            :     Dwarf_Block 
DW_DLA_DEBUG            :     Dwarf_Debug (do not use)
DW_DLA_DIE              :     Dwarf_Die
DW_DLA_LINE             :     Dwarf_Line 
DW_DLA_ATTR             :     Dwarf_Attribute 
DW_DLA_TYPE             :     Dwarf_Type  (not used) 
DW_DLA_SUBSCR           :     Dwarf_Subscr (not used) 
DW_DLA_GLOBAL           :     Dwarf_Global 
DW_DLA_ERROR            :     Dwarf_Error 
DW_DLA_LIST             :     a list of opaque descriptors
DW_DLA_LINEBUF          :     Dwarf_Line* (not used) 
DW_DLA_ARANGE           :     Dwarf_Arange 
DW_DLA_ABBREV           :     Dwarf_Abbrev 
DW_DLA_FRAME_OP         :     Dwarf_Frame_Op 
DW_DLA_CIE              :     Dwarf_Cie 
DW_DLA_FDE              :     Dwarf_Fde
DW_DLA_LOC_BLOCK        :     Dwarf_Loc Block
DW_DLA_FRAME_BLOCK      :     Dwarf_Frame Block (not used) 
DW_DLA_FUNC             :     Dwarf_Func 
DW_DLA_TYPENAME         :     Dwarf_Type
DW_DLA_VAR              :     Dwarf_Var
DW_DLA_WEAK		:     Dwarf_Weak
.TE
.FG "Allocation/Deallocation Identifiers"
.DE

.P
.H 1 "Functional Interface"
This section describes the functions available in the \fIlibdwarf\fP
library.  Each function description includes its definition, followed 
by one or more paragraph describing the function's operation.

.P
The following sections describe these functions.

.H 2 "Initialization Operations"
These functions are concerned with preparing an object file for subsequent
access by the functions in \fIlibdwarf\fP and with releasing allocated
resources when access is complete. 

.H 3 "dwarf_init()"

.DS
\f(CWint dwarf_init(
        int fd,
        Dwarf_Unsigned access,
        Dwarf_Handler errhand, 
        Dwarf_Ptr errarg,
	Dwarf_Debug * dbg,
        Dwarf_Error *error)\fP
.DE
When it returns \f(CWDW_DLV_OK\fP,
the function \f(CWdwarf_init()\fP returns  thru
\f(CWdbg\fP a \f(CWDwarf_Debug\fP descriptor 
that represents a handle for accessing debugging records associated with 
the open file descriptor \f(CWfd\fP.  
\f(CWDW_DLV_NO_ENTRY\fP is returned if the object
does not contain DWARF debugging information.
\f(CWDW_DLV_ERROR\fP is returned if
an error occurred.
The 
\f(CWaccess\fP argument indicates what access is allowed for the section. 
The \f(CWDW_DLC_READ\fP parameter is valid
for read access (only read access is defined or discussed in this
document).  
The \f(CWerrhand\fP 
argument is a pointer to a function that will be invoked whenever an error 
is detected as a result of a \fIlibdwarf\fP operation.  The \f(CWerrarg\fP 
argument is passed as an argument to the \f(CWerrhand\fP function.  
The file 
descriptor associated with the \f(CWfd\fP argument must refer to an ordinary 
file (i.e. not a pipe, socket, device, /proc entry, etc.), be opened with 
the at least as much permission as specified by the \f(CWaccess\fP argument, 
and cannot be closed or used as an argument to any system calls by the 
client until after \f(CWdwarf_finish()\fP is called.  
The seek position of 
the file associated with \f(CWfd\fP is undefined upon return of 
\f(CWdwarf_init()\fP.

Since \f(CWdwarf_init()\fP uses the same error handling processing as other 
\fIlibdwarf\fP functions (see \fIError Handling\fP above), client programs 
will generally supply an \f(CWerror\fP parameter to bypass the default actions 
during initialization unless the default actions are appropriate. 

.H 3 "dwarf_elf_init()"
.DS
\f(CWint dwarf_elf_init(
        Elf * elf_file_pointer,
        Dwarf_Unsigned access,
        Dwarf_Handler errhand, 
        Dwarf_Ptr errarg,
	Dwarf_Debug * dbg,
        Dwarf_Error *error)\fP
.DE
The function \f(CWdwarf_elf_init()\fP is identical to \f(CWdwarf_init()\fP 
except that an open \f(CWElf *\fP pointer is passed instead of a file 
descriptor.  
In systems supporting \f(CWELF\fP object files this may be 
more space or time-efficient than using \f(CWdwarf_init()\fP.
The client is allowed to use the \f(CWElf *\fP pointer
for its own purposes without restriction during the time the 
\f(CWDwarf_Debug\fP
is open, except that the client should not  \f(CWelf_end()\fP the
pointer till after  \f(CWdwarf_finish\fP is called.

.H 3 "dwarf_get_elf()"
.DS
\f(CWint dwarf_get_elf(
        Dwarf_Debug dbg,
        Elf **      elf,
        Dwarf_Error *error)\fP
.DE
When it returns \f(CWDW_DLV_OK\fP,
the function \f(CWdwarf_get_elf()\fP returns thru the
pointer \f(CWelf\fP the \f(CWElf *\fP handle
used to access the object represented by the \f(CWDwarf_Debug\fP
descriptor \f(CWdbg\fP.  It returns \f(CWDW_DLV_ERROR\fP on error.

Because \f(CWint dwarf_init()\fP opens an Elf descriptor
on its fd and \f(CWdwarf_finish()\fP does not close that
descriptor, an app should use \f(CWdwarf_get_elf\fP
and should call \f(CWelf_end\fP with the pointer returned
thru the \f(CWElf**\fP handle created by \f(CWint dwarf_init()\fP.

This function is not meaningful for a system that does not used the
Elf format for objects.

.H 3 "dwarf_finish()"
.DS
\f(CWint dwarf_finish(
        Dwarf_Debug dbg,
	Dwarf_Error *error)\fP
.DE
The function
\f(CWdwarf_finish()\fP releases all \fILibdwarf\fP internal resources 
associated with the descriptor \f(CWdbg\fP, and invalidates \f(CWdbg\fP.  
It returns \f(CWDW_DLV_ERROR\fP if there is an error during the
finishing operation.  It returns \f(CWDW_DLV_OK\fP 
for a successful operation.

Because \f(CWint dwarf_init()\fP opens an Elf descriptor
on its fd and \f(CWdwarf_finish()\fP does not close that
descriptor, an app should use \f(CWdwarf_get_elf\fP
and should call \f(CWelf_end\fP with the pointer returned
thru the \f(CWElf**\fP handle created by \f(CWint dwarf_init()\fP.

.H 2 "Debugging Information Entry Delivery Operations"
These functions are concerned with accessing debugging information 
entries. 

.H 3 "Debugging Information Entry Debugger Delivery Operations"

.H 3 "dwarf_next_cu_header()"
.DS
\f(CWint dwarf_next_cu_header(
        Dwarf_debug dbg,
        Dwarf_Unsigned *cu_header_length,
        Dwarf_Half     *version_stamp,
        Dwarf_Unsigned *abbrev_offset,
        Dwarf_Half     *address_size,
        Dwarf_Unsigned *next_cu_header,
        Dwarf_Error    *error);
.DE
The function
\f(CWdwarf_next_cu_header()\fP returns \f(CWDW_DLV_ERROR\fP 
if it fails, and
\f(CWDW_DLV_OK\fP if it succeeds.
.P
If it succeeds, \f(CW*next_cu_header\fP is set to
the offset in the .debug_info section of the next 
compilation-unit header if it succeeds.  On reading the last 
compilation-unit header in the .debug_info section it contains 
the size of the .debug_info section.
The next call to 
\f(CWdwarf_next_cu_header()\fP returns \f(CWDW_DLV_NO_ENTRY\fP
without reading a 
compilation-unit or setting \f(CW*next_cu_header\fP.  
Subsequent calls to \f(CWdwarf_next_cu_header()\fP 
repeat the cycle by reading the first compilation-unit and so on.  
.P
The other 
values returned through pointers are the values in the compilation-unit 
header.  If any of \f(CWcu_header_length\fP, \f(CWversion_stamp\fP,
\f(CWabbrev_offset\fP, or \f(CWaddress_size\fP is \f(CWNULL\fP, the 
argument is ignored (meaning it is not an error to provide a 
\f(CWNULL\fP pointer).

.H 3 "dwarf_siblingof()"
.DS
\f(CWint dwarf_siblingof(
        Dwarf_Debug dbg, 
        Dwarf_Die die, 
	Dwarf_Die *return_sib,
        Dwarf_Error *error)\fP
.DE
The function \f(CWdwarf_siblingof()\fP 
returns \f(CWDW_DLV_ERROR\fP and sets the \f(CWerror\fP pointer on error.
If there is no sibling it returns \f(CWDW_DLV_NO_ENTRY\fP.
When it succeeds,
\f(CWdwarf_siblingof()\fP returns
\f(CWDW_DLV_OK\fP  and sets \f(CW*return_sib\fP to the \f(CWDwarf_Die\fP 
descriptor of the sibling of \f(CWdie\fP.
If \f(CWdie\fP is \fINULL\fP, the \f(CWDwarf_Die\fP descriptor of the
first die in the compilation-unit is returned.  
This die has the
\f(CWDW_TAG_compile_unit\fP tag.
.H 3 "dwarf_child()"
.DS
\f(CWint dwarf_child(
        Dwarf_Die die, 
	Dwarf_Die *return_kid,
        Dwarf_Error *error)\fP
.DE
The function \f(CWdwarf_child()\fP 
returns \f(CWDW_DLV_ERROR\fP and sets the \f(CWerror\fP die on error.
If there is no child it returns \f(CWDW_DLV_NO_ENTRY\fP.
When it succeeds,
\f(CWdwarf_child()\fP returns 
\f(CWDW_DLV_OK\fP and sets \f(CW*return_kid\fP
to the \f(CWDwarf_Die\fP descriptor 
of the first child of \f(CWdie\fP.
The function 
\f(CWdwarf_siblingof()\fP can be used with the return value of 
\f(CWdwarf_child()\fP to access the other children of \f(CWdie\fP. 

.H 3 "dwarf_offdie()"
.DS
\f(CWint dwarf_offdie(
        Dwarf_Debug dbg,
        Dwarf_Off offset, 
	Dwarf_Die *return_die,
        Dwarf_Error *error)\fP
.DE
The function \f(CWdwarf_offdie()\fP 
returns \f(CWDW_DLV_ERROR\fP and sets the \f(CWerror\fP die on error.
When it succeeds,
\f(CWdwarf_offdie()\fP returns
\f(CWDW_DLV_OK\fP and sets \f(CW*return_die\fP
to the
the \f(CWDwarf_Die\fP 
descriptor of the debugging information entry at \f(CWoffset\fP in 
the section containing debugging information entries i.e the .debug_info
section.  
It is the user's 
responsibility to make sure that \f(CWoffset\fP is the start of a valid 
debugging information entry.  The result of passing it an invalid 
offset could be chaos.

.\"#if 0
.\".H 3 "Debugging Entry Delivery High-level Operations"
.\"The following "higher level" operations are typically not used by 
.\"debuggers or DWARF prettyprinters.  A disassembler (for example) 
.\"might find them useful.
.\"
.\".DS
.\"\f(CWDwarf_Die dwarf_pcfile(
.\"        Dwarf_Debug dbg, 
.\"        Dwarf_Addr pc, 
.\"        Dwarf_Error *error)\fP
.\".DE
.\"The function \f(CWdwarf_pcfile()\fP returns the \f(CWDwarf_Die\fP 
.\"descriptor of the compilation unit debugging information entry that 
.\"contains the address of \f(CWpc\fP.  It returns \fINULL\fP if no 
.\"entry exists or an error occurred.  Currently compilation unit 
.\"debugging information entries are defined as those having a tag of: 
.\"\f(CWDW_TAG_compile_unit\fP.  This function is currently unimplemented.
.\"
.\".DS
.\"\f(CWDwarf_Die dwarf_pcsubr(
.\"        Dwarf_Debug dbg, 
.\"        Dwarf_Addr pc, 
.\"        Dwarf_Error *error)\fP
.\".DE
.\"The function
.\"\f(CWdwarf_pcsubr()\fP returns the \f(CWDwarf_Die\fP descriptor of the 
.\"subroutine debugging entry that contains the address of \f(CWpc\fP.  It 
.\"returns \fINULL\fP if no entry exists or an error occurred.  Currently 
.\"subroutine debugging information entries are defined as those having a 
.\"tag of: \f(CWDW_TAG_subprogram\fP, or \f(CWTAG_inlined_subroutine\fP.
.\"This function is currently unimplemented.
.\"
.\".DS
.\"\f(CWDwarf_Die dwarf_pcscope(
.\"        Dwarf_Debug dbg, 
.\"        Dwarf_Addr pc, 
.\"        Dwarf_Error *error)\fP
.\".DE
.\"The function
.\"\f(CWdwarf_pcscope()\fP returns the \f(CWDwarf_Die\fP descriptor for 
.\"the debugging information entry that represents the innermost enclosing 
.\"scope containing \f(CWpc\fP, or \fINULL\fP if no entry exists or an 
.\"error occurred. Debugging information entries that represent a \fIscope\fP 
.\"are those containing a low pc attribute and either a high pc or byte 
.\"size attribute that delineates a range. For example: a debugging information 
.\"entry for a lexical block is considered one having a scope whereas a 
.\"debugging information entry for a label is not.  This function is
.\"currently unimplemented.
.\"#endif


.H 2 "Debugging Information Entry Query Operations"
These queries return specific information about debugging information 
entries or a descriptor that can be used on subsequent queries when 
given a \f(CWDwarf_Die\fP descriptor.  Note that some operations are 
specific to debugging information entries that are represented by a 
\f(CWDwarf_Die\fP descriptor of a specific type. 
For example, not all 
debugging information entries contain an attribute having a name, so 
consequently, a call to \f(CWdwarf_diename()\fP using a \f(CWDwarf_Die\fP 
descriptor that does not have a name attribute will return
\f(CWDW_DLV_NO_ENTRY\fP.
This is not an error, i.e. calling a function that needs a specific
attribute is not an error for a die that does not contain that specific
attribute.
.P
There are several methods that can be used to obtain the value of an
attribute in a given die:
.AL 1
.LI
Call \f(CWdwarf_hasattr()\fP to determine if the debugging information
entry has the attribute of interest prior to issuing the query for
information about the attribute.

.LI
Supply an \f(CWerror\fP argument, and check its value after the call to 
a query indicates an unsuccessful return, to determine the nature of the 
problem.  The \f(CWerror\fP argument will indicate whether an error occurred, 
or the specific attribute needed was missing in that die.

.LI
Arrange to have an error handling function invoked upon detection of an 
error (see \f(CWdwarf_init()\fP).

.LI
Call \f(CWdwarf_attrlist()\fP and iterate through the returned list of
attributes, dealing with each one as appropriate.
.LE
.P

.H 3 "dwarf_tag()"
.DS
\f(CWint dwarf_tag(
        Dwarf_Die die, 
	Dwarf_Half *tagval,
        Dwarf_Error *error)\fP
.DE
The function \f(CWdwarf_tag()\fP returns the \fItag\fP of \f(CWdie\fP
thru the pointer  \f(CWtagval\fP if it succeeds. 
It returns \f(CWDW_DLV_OK\fP if it succeeds.
It returns \f(CWDW_DLV_ERROR\fP on error.

.H 3 "dwarf_dieoffset()"
.DS
\f(CWint dwarf_dieoffset(
        Dwarf_Die die, 
	Dwarf_Off * return_offset,
        Dwarf_Error *error)\fP
.DE
When it succeeds,
the function \f(CWdwarf_dieoffset()\fP returns 
\f(CWDW_DLV_OK\fP and sets \f(CW*return_offset\fP
to the position of \f(CWdie\fP 
in the section containing debugging information entries
(the \f(CWreturn_offset\fP is a section-relative offset).  
In other words,
it sets \f(CWreturn_offset\fP 
to the offset of the start of the debugging information entry
described by \f(CWdie\fP in the section containing die's i.e .debug_info.  
It returns \f(CWDW_DLV_ERROR\fP on error.

.H 3 "dwarf_die_CU_offset()"
.DS
\f(CWint dwarf_die_CU_offset(
        Dwarf_Die die,
  	Dwarf_Off *return_offset,
        Dwarf_Error *error)\fP
.DE
The function \f(CWdwarf_die_CU_offset()\fP is similar to 
\f(CWdwarf_dieoffset()\fP, except that it puts the offset of the DIE 
represented by the \f(CWDwarf_Die\fP \f(CWdie\fP, from the 
start of the compilation-unit that it belongs to rather than the start 
of .debug_info (the \f(CWreturn_offset\fP is a CU-relative offset).  


.H 3 "dwarf_diename()"
.DS
\f(CWint dwarf_diename(
        Dwarf_Die die, 
	char  ** return_name,
        Dwarf_Error *error)\fP
.DE
When it succeeds,
the function \f(CWdwarf_diename()\fP returns
\f(CWDW_DLV_OK\fP and sets \f(CW*return_name\fP
to
a pointer to a
null-terminated string of characters that represents the name
attribute of \f(CWdie\fP.
It returns \f(CWDW_DLV_NO_ENTRY\fP if \f(CWdie\fP does not have a name attribute.
It returns \f(CWDW_DLV_ERROR\fP if
an error occurred.  
The storage pointed to by a successful return of 
\f(CWdwarf_diename()\fP should be free'd using the allocation type
\f(CWDW_DLA_STRING\fP when no longer of interest (see 
\f(CWdwarf_dealloc()\fP).

.H 3 "dwarf_attrlist()"
.DS
\f(CWint dwarf_attrlist(
        Dwarf_Die die, 
        Dwarf_Attribute** attrbuf, 
	Dwarf_Signed *attrcount,
        Dwarf_Error *error)\fP
.DE
When it returns \f(CWDW_DLV_OK\fP,
the function \f(CWdwarf_attrlist()\fP sets \f(CWattrbuf\fP to point 
to an array of \f(CWDwarf_Attribute\fP descriptors corresponding to
each of the attributes in die, and returns the number of elements in 
the array thru \f(CWattrcount\fP.  
\f(CWDW_DLV_NO_ENTRY\fP is returned if the count is zero (no 
\f(CWattrbuf\fP is allocated in this case).
\f(CWDW_DLV_ERROR\fP is returned on error.
On a successful return from \f(CWdwarf_attrlist()\fP, each of the
\f(CWDwarf_Attribute\fP descriptors should be individually free'd using 
\f(CWdwarf_dealloc()\fP with the allocation type \f(CWDW_DLA_ATTR\fP, 
followed by free-ing the list pointed to by \f(CW*attrbuf\fP using
\f(CWdwarf_dealloc()\fP with the allocation type \f(CWDW_DLA_LIST\fP, 
when no longer of interest (see \f(CWdwarf_dealloc()\fP).

Freeing the attrlist:
.in +2
.DS
\f(CWDwarf_Unsigned atcnt;
Dwarf_Attribute *atlist;
int errv;

if ((errv = dwarf_attrlist(somedie, &atlist,&atcnt, &error)) == DW_DLV_OK) {

        for (i = 0; i < atcnt; ++i) {
                /* use atlist[i] */
                dwarf_dealloc(dbg, atlist[i], DW_DLA_ATTR);
        }
        dwarf_dealloc(dbg, atlist, DW_DLA_LIST);
}\fP
.DE
.in -2
.P
.H 3 "dwarf_hasattr()"
.DS
\f(CWint dwarf_hasattr(
        Dwarf_Die die, 
        Dwarf_Half attr, 
	Dwarf_Bool *return_bool,
        Dwarf_Error *error)\fP
.DE
When it succeeds, the
function \f(CWdwarf_hasattr()\fP returns \f(CWDW_DLV_OK\fP
and sets \f(CW*return_bool\fP to \fInon-zero\fP if 
\f(CWdie\fP has the attribute \f(CWattr\fP and \fIzero\fP otherwise.
If it fails, it returns \f(CWDW_DLV_ERROR\fP.

.H 3 "dwarf_attr()"
.DS
\f(CWint dwarf_attr(
        Dwarf_Die die, 
        Dwarf_Half attr, 
	Dwarf_Attribute *return_attr,
        Dwarf_Error *error)\fP
.DE
When it returns \f(CWDW_DLV_OK\fP,
the function \f(CWdwarf_attr()\fP
sets 
\f(CW*return_attr\fP to the  \f(CWDwarf_Attribute\fP 
descriptor of \f(CWdie\fP having the attribute \f(CWattr\fP.
It returns \f(CDW_DLV_NO_ENTRY\fP if \f(CWattr\fP is not contained 
in \f(CWdie\fP. 
It returns \f(CDW_DLV_ERROR\fP if an error occurred.

.\"#if 0
.\".DS
.\"\f(CWDwarf_Locdesc* dwarf_stringlen(
.\"        Dwarf_Die die, 
.\"        Dwarf_Error *error)\fP
.\".DE
.\"The function \f(CWdwarf_stringlen()\fP returns a pointer to a 
.\"\f(CWDwarf_Locdesc\fP with one Locdesc entry that when evaluated,
.\"yields the length of the string represented by \f(CWdie\fP.  It
.\"returns \f(CWNULL\fP if \f(CWdie\fP does not contain a string length 
.\"attribute or the string length attribute is not a location-description 
.\"or an error occurred. The address range of the list is set to 0 thru 
.\"the highest possible address if a loclist pointer is returned.  The 
.\"storage pointed to by a successful return of \f(CWdwarf_stringlen()\fP 
.\"should be free'd when no longer of interest (see \f(CWdwarf_dealloc()\fP).
.\"This function is currently unimplemented.
.\"#endif

.\"#if 0
.\".DS
.\"\f(CWDwarf_Signed dwarf_subscrcnt(
.\"        Dwarf_Die die, 
.\"        Dwarf_Error *error)\fP
.\".DE
.\"The function \f(CWdwarf_subscrcnt()\fP returns the number of subscript 
.\"die's that are owned by the array type represented by \f(CWdie\fP.  It
.\"returns \f(CWDW_DLV_NOCOUNT\fP on error.  This function is currently
.\"unimplemented.
.\"
.\".DS
.\"\f(CWDwarf_Die dwarf_nthsubscr(
.\"        Dwarf_Die die, 
.\"        Dwarf_Unsigned ssndx, 
.\"        Dwarf_Error *error)\fP
.\".DE
.\"The function \f(CWdwarf_nthsubscr()\fP returns a \f(CWDwarf_Die\fP 
.\"descriptor that describes the \f(CWssndx\fP subscript of the array 
.\"type debugging information entry represented by \f(CWdie\fP, where 
.\"\fI1\fP is the first member.  It returns \fINULL\fP if \f(CWdie\fP 
.\"does not have an \f(CWssndx\fP subscript, or an error occurred.  
.\"This function is currently unimplemented.
.\"#endif

.H 3 "dwarf_lowpc()"
.DS
\f(CWint dwarf_lowpc(
        Dwarf_Die     die, 
	Dwarf_Addr  * return_lowpc,
        Dwarf_Error * error)\fP
.DE
The function \f(CWdwarf_lowpc()\fP returns
\f(CWDW_DLV_OK\fP and sets \f(CW*return_lowpc\fP
to the low program counter 
value associated with the \f(CWdie\fP descriptor if \f(CWdie\fP 
represents a debugging information entry with this attribute.  
It returns \f(CWDW_DLV_NO_ENTRY\fP if \f(CWdie\fP does not have this 
attribute. 
It returns \f(CWDW_DLV_ERROR\fP if an error occurred. 

.H 3 "dwarf_highpc()"
.DS
\f(CWint dwarf_highpc(
        Dwarf_Die die, 
	Dwarf_Addr  * return_highpc,
        Dwarf_Error *error)\fP
.DE
The function \f(CWdwarf_highpc()\fP returns 
\f(CWDW_DLV_OK\fP and sets \f(CW*return_highpc\fP
the high program counter 
value associated with the \f(CWdie\fP descriptor if \f(CWdie\fP 
represents a debugging information entry with this attribute.  
It returns \f(CWDW_DLV_NO_ENTRY\fP if \f(CWdie\fP does not have this 
attribute. 
It returns \f(CWDW_DLV_ERROR\fP if an error occurred. 

.H 3 "dwarf_bytesize()"
.DS
\f(CWDwarf_Signed dwarf_bytesize(
        Dwarf_Die        die, 
	Dwarf_Unsigned  *return_size,
        Dwarf_Error     *error)\fP
.DE
When it succeeds,
\f(CWdwarf_bytesize()\fP returns 
\f(CWDW_DLV_OK\fP and sets \f(CW*return_size\fP
to the number of bytes 
needed to contain an instance of the aggregate debugging information 
entry represented by \f(CWdie\fP.  
It returns \f(CWDW_DLV_NO_ENTRY\fP if 
\f(CWdie\fP does not contain the byte size attribute \f(CWDW_AT_byte_size\fP.
It returns \f(CWDW_DLV_ERROR\fP if 
an error occurred.

.\"#if 0
.\".DS
.\"\f(CWDwarf_Bool dwarf_isbitfield(
.\"        Dwarf_Die die, 
.\"        Dwarf_Error *error)\fP
.\".DE
.\"The function \f(CWdwarf_isbitfield()\fP returns \fInon-zero\fP if 
.\"\f(CWdie\fP is a descriptor for a debugging information entry that 
.\"represents a bit field member.  It returns \fIzero\fP if \f(CWdie\fP 
.\"is not associated with a bit field member.  This function is currently
.\"unimplemented.
.\"#endif

.H 3 "dwarf_bitsize()"
.DS
\f(CWint dwarf_bitsize(
        Dwarf_Die die, 
	Dwarf_Unsigned  *return_size,
        Dwarf_Error *error)\fP
.DE
When it succeeds,
\f(CWdwarf_bitsize()\fP returns 
\f(CWDW_DLV_OK\fP and sets \f(CW*return_size\fP
to the number of 
bits 
occupied by the bit field value that is an attribute of the given
die.  
It returns \f(CWDW_DLV_NO_ENTRY\fP if \f(CWdie\fP does not 
contain the bit size attribute \f(CWDW_AT_bit_size\fP.
It returns \f(CWDW_DLV_ERROR\fP if 
an error occurred.

.H 3 "dwarf_bitoffset()"
.DS
\f(CWint dwarf_bitoffset(
        Dwarf_Die die, 
	Dwarf_Unsigned  *return_size,
        Dwarf_Error *error)\fP
.DE
When it succeeds,
\f(CWdwarf_bitoffset()\fP returns 
\f(CWDW_DLV_OK\fP and sets \f(CW*return_size\fP
to the number of bits 
to the left of the most significant bit of the bit field value. 
This bit offset is not necessarily the net bit offset within the
structure or class , since \f(CWDW_AT_data_member_location\fP
may give a byte offset to this \f(CWDIE\fP and the bit offset
returned through the pointer
does not include the bits in the byte offset.
It returns \f(CWDW_DLV_NO_ENTRY\fP if \f(CWdie\fP does not contain the 
bit offset attribute \f(CWDW_AT_bit_offset\fP.
It returns \f(CWDW_DLV_ERROR\fP if 
an error occurred.

.H 3 "dwarf_srclang()"
.DS
\f(CWint dwarf_srclang(
        Dwarf_Die die, 
	Dwarf_Unsigned  *return_lang,
        Dwarf_Error *error)\fP
.DE
When it succeeds,
\f(CWdwarf_srclang()\fP returns
\f(CWDW_DLV_OK\fP and sets \f(CW*return_lang\fP
to
a code indicating the 
source language of the compilation unit represented by the descriptor 
\f(CWdie\fP.  
It returns \f(CWDW_DLV_NO_ENTRY\fP if \f(CWdie\fP does not 
represent a source file debugging information entry (i.e. contain the 
attribute \f(CWDW_AT_language\fP).
It returns \f(CWDW_DLV_ERROR\fP if 
an error occurred.

.H 3 "dwarf_arrayorder()"
.DS
\f(CWint dwarf_arrayorder(
        Dwarf_Die die, 
	Dwarf_Unsigned  *return_order,
        Dwarf_Error *error)\fP
.DE
When it succeeds,
\f(CWdwarf_arrayorder()\fP returns
\f(CWDW_DLV_OK\fP and sets \f(CW*return_order\fP
a code indicating 
the ordering of the array represented by the descriptor \f(CWdie\fP.
It returns \f(CWDW_DLV_NO_ENTRY\fP if \f(CWdie\fP does not contain the
array order attribute \f(CWDW_AT_ordering\fP.
It returns \f(CWDW_DLV_ERROR\fP if 
an error occurred.

.H 2 "Attribute Form Queries"
Based on the attribute's form, these operations are concerned with 
returning uninterpreted attribute data.  Since it is not always 
obvious from the return value of these functions if an error occurred, 
one should always supply an \f(CWerror\fP parameter or have arranged 
to have an error handling function invoked (see \f(CWdwarf_init()\fP)
to determine the validity of the returned value and the nature of any 
errors that may have occurred.

A \f(CWDwarf_Attribute\fP descriptor describes an attribute of a
specific die.  Thus, each \f(CWDwarf_Attribute\fP descriptor is
implicitly associated with a specific die.

.H 3 "dwarf_hasform()"
.DS
\f(CWnt dwarf_hasform(
        Dwarf_Attribute attr, 
        Dwarf_Half form, 
        Dwarf_Bool  *return_hasform,
        Dwarf_Error *error)\fP
.DE
The function \f(CWdwarf_hasform()\fP returns 
\f(CWDW_DLV_OK\fP and  and puts a 
\fInon-zero\fP
 value in the 
\f(CW*return_hasform\fP boolean if the 
attribute represented by the \f(CWDwarf_Attribute\fP descriptor 
\f(CWattr\fP has the attribute form \f(CWform\fP.  
If the attribute does not have that form \fIzero\fP
is put into \f(CW*return_hasform\fP. 
\f(CWDW_DLV_ERROR\fP is returned on error.

.H 3 "dwarf_whatform()"
.DS
\f(CWint dwarf_whatform(
        Dwarf_Attribute attr,
        Dwarf_Half     *return_form,
        Dwarf_Error *error)\fP
.DE
When it succeeds,
\f(CWdwarf_whatform()\fP returns 
\f(CWDW_DLV_OK\fP and sets \f(CW*return_form\fP
to the attribute form code of 
the attribute represented by the \f(CWDwarf_Attribute\fP descriptor 
\f(CWattr\fP.  
It returns  \f(CWDW_DLV_ERROR\fP  on error.

.H 3 "dwarf_whatattr()"
.DS
\f(CWint dwarf_whatattr(
        Dwarf_Attribute attr,
        Dwarf_Half     *return_attr,
        Dwarf_Error *error)\fP
.DE
When it succeeds,
\f(CWdwarf_whatattr()\fP returns
\f(CWDW_DLV_OK\fP and sets \f(CW*return_attr\fP
to the attribute code 
represented by the \f(CWDwarf_Attribute\fP descriptor \f(CWattr\fP.  
It returns  \f(CWDW_DLV_ERROR\fP  on error.

.H 3 "dwarf_formref()"
.DS
\f(CWint dwarf_formref(
        Dwarf_Attribute attr, 
	Dwarf_Off     *return_offset,
        Dwarf_Error *error)\fP
.DE
When it succeeds,
\f(CWdwarf_formref()\fP returns 
\f(CWDW_DLV_OK\fP and sets \f(CW*return_offset\fP
to the CU-relative offset
represented by the descriptor \f(CWattr\fP if the form of the attribute 
belongs to the \f(CWREFERENCE\fP class.
\f(CWattr\fP must be a CU-local reference, 
not form \f(CWDW_FORM_ref_addr\fP.  
It is an error for the form to
not belong to this class or to be form \f(CWDW_FORM_ref_addr\fP.  
It returns \f(CWDW_DLV_ERROR\fP on error.
See also \f(CWdwarf_global_formref\fP below.

.H 3 "dwarf_global_formref()"
.DS
\f(CWint dwarf_global_formref(
        Dwarf_Attribute attr, 
	Dwarf_Off     *return_offset,
        Dwarf_Error *error)\fP
.DE
When it succeeds,
\f(CWdwarf_global_formref()\fP returns 
\f(CWDW_DLV_OK\fP and sets \f(CW*return_offset\fP
to the .debug_info-section-relative offset
represented by the descriptor \f(CWattr\fP if the form of the attribute 
belongs to the \f(CWREFERENCE\fP class. 
\f(CWattr\fP can be any legal 
\f(CWREFERENCE\fP class form including \f(CWDW_FORM_ref_addr\fP.
It is an error for the form to
not belong to this class.
It returns \f(CWDW_DLV_ERROR\fP on error.
See also \f(CWdwarf_formref\fP above.

.H 3 "dwarf_formaddr()"
.DS
\f(CWint dwarf_formaddr(
        Dwarf_Attribute attr, 
        Dwarf_Addr    * return_addr,
        Dwarf_Error *error)\fP
.DE
When it succeeds,
\f(CWdwarf_formaddr()\fP returns 
\f(CWDW_DLV_OK\fP and sets \f(CW*return_addr\fP
to
the address 
represented by the descriptor \f(CWattr\fP if the form of the attribute
belongs to the \f(CWADDRESS\fP class.  
It is an error for the form to
not belong to this class.  
It returns \f(CWDW_DLV_ERROR\fP on error.

.H 3 "dwarf_formflag()"
.DS
\f(CWint dwarf_formflag(
	Dwarf_Attribute attr,
	Dwarf_Bool * return_bool,
	Dwarf_Error *error)\fP
.DE
When it succeeds,
\f(CWdwarf_formflag()\fP returns
\f(CWDW_DLV_OK\fP and sets \f(CW*return_bool\fP
\f(CW1\fP (i.e. true) (if the attribute has a non-zero value)
or \f(CW0\fP (i.e. false) (if the attribute has a zero value).
It returns \f(CWDW_DLV_ERROR\fP on error or if the \f(CWattr\fP
does not have form flag.

.H 3 "dwarf_formudata()"
.DS
\f(CWint dwarf_formudata(
        Dwarf_Attribute   attr, 
	Dwarf_Unsigned  * return_uvalue,
        Dwarf_Error     * error)\fP
.DE
The function
\f(CWdwarf_formudata()\fP returns 
\f(CWDW_DLV_OK\fP and sets \f(CW*return_uvalue\fP
to
the \f(CWDwarf_Unsigned\fP 
value of the attribute represented by the descriptor \f(CWattr\fP if the
form of the attribute belongs to the \f(CWCONSTANT\fP class.  
It is an 
error for the form to not belong to this class.  
It returns \f(CWDW_DLV_ERROR\fP on error.

.H 3 "dwarf_formsdata()"
.DS
\f(CWint dwarf_formsdata(
        Dwarf_Attribute attr, 
	Dwarf_Signed  * return_svalue,
        Dwarf_Error *error)\fP
.DE
The function \f(CWdwarf_formsdata()\fP returns 
\f(CWDW_DLV_OK\fP and sets \f(CW*return_svalue\fP
to
the \f(CWDwarf_Signed\fP 
value of the attribute represented by the descriptor \f(CWattr\fP if the
form of the attribute belongs to the \f(CWCONSTANT\fP class.  
It is an 
error for the form to not belong to this class.  
If the size of the data 
attribute referenced is smaller than the size of the \f(CWDwarf_Signed\fP
type, its value is sign extended.  
It returns \f(CWDW_DLV_ERROR\fP on error.

.H 3 "dwarf_formblock()"
.DS
\f(CWint dwarf_formblock(
        Dwarf_Attribute attr, 
	Dwarf_Block  ** return_block,
        Dwarf_Error *   error)\fP
.DE
The function \f(CWdwarf_formblock()\fP returns 
\f(CWDW_DLV_OK\fP and sets \f(CW*return_block\fP
to
a pointer to a 
\f(CWDwarf_Block\fP structure containing the value of the attribute 
represented by the descriptor \f(CWattr\fP if the form of the 
attribute belongs to the \f(CWBLOCK\fP class.  
It is an error
for the form to not belong to this class.  
The storage pointed 
to by a successful return of \f(CWdwarf_formblock()\fP should 
be free'd using the allocation type \f(CWDW_DLA_BLOCK\fP,  when 
no longer of interest (see \f(CWdwarf_dealloc()\fP).  
It returns
\f(CWDW_DLV_ERROR\fP on error.

.H 3 "dwarf_formstring()"

.DS
\f(CWint dwarf_formstring(
        Dwarf_Attribute attr, 
	char        **  return_string,
        Dwarf_Error *error)\fP
.DE
The function \f(CWdwarf_formstring()\fP returns 
\f(CWDW_DLV_OK\fP and sets \f(CW*return_string\fP
to
a pointer to a 
null-terminated string containing  the value of the attribute 
represented by the descriptor \f(CWattr\fP if the form of the
attribute belongs to the \f(CWSTRING\fP class.  
It is an error
for the form to not belong to this class.  
The storage pointed 
to by a successful return of \f(CWdwarf_formstring()\fP should 
be free'd using the allocation type \f(CWDW_DLA_STRING\fP when 
no longer of interest (see \f(CWdwarf_dealloc()\fP).  
It returns \f(CWDW_DLV_ERROR\fP on error.

.H 4 "dwarf_loclist()"
.DS
\f(CWint dwarf_loclist(
        Dwarf_Attribute attr, 
        Dwarf_Locdesc **llbuf,
        Dwarf_Signed  *listlen,
        Dwarf_Error *error)\fP
.DE
The function \f(CWdwarf_loclist()\fP sets \f(CW*llbuf\fP to point to 
an array of \f(CWDwarf_Locdesc\fP pointers corresponding to each of
the location expressions in a location list, and sets
\f(CW*listlen\fP to the number 
of elements in the array and returns \f(CWDW_DLV_OK\fP if the attribute is
appropriate.
It returns \f(CWDW_DLV_ERROR\fP on error. 
\f(CWdwarf_loclist()\fP works on \f(CWDW_AT_location\fP, 
\f(CWDW_AT_data_member_location\fP, \f(CWDW_AT_vtable_elem_location\fP,
\f(CWDW_AT_string_length\fP, \f(CWDW_AT_use_location\fP, and 
\f(CWDW_AT_return_addr\fP attributes.  

Storage allocated by a successful call of \f(CWdwarf_loclist()\fP should 
be deallocated when no longer of interest (see \f(CWdwarf_dealloc()\fP).
The block of \f(CWDwarf_Loc\fP structs pointed to by the \f(CWld_s\fP 
field of each \f(CWDwarf_Locdesc\fP structure 
should be deallocated with the allocation type \f(CWDW_DLA_LOC_BLOCK\fP. 
This should be followed by deallocation of the \f(CWllbuf\fP
using the allocation type \f(CWDW_DLA_LOCDESC\fP.
.in +2
.DS
\f(CWDwarf_Signed lcnt;
Dwarf_Locdesc *llbuf;
int lres;

if ((lres = dwarf_loclist(someattr, &llbuf,&lcnt &error)) == DW_DLV_OK) {
        for (i = 0; i < lcnt; ++i) {
            /* use llbuf[i] */

            /* Deallocate Dwarf_Loc block of llbuf[i] */
            dwarf_dealloc(dbg, llbuf[i].ld_s, DW_DLA_LOC_BLOCK);

        }
        dwarf_dealloc(dbg, llbuf, DW_DLA_LOCDESC);
}\fP
.DE
.in -2
.P

.P
.H 2 "Line Number Operations"
These functions are concerned with accessing line number entries,
mapping debugging information entry objects to their corresponding
source lines, and providing a mechanism for obtaining information
about line number entries.  Although, the interface talks of "lines"
what is really meant is "statements".  In case there is more than
one statement on the same line, there will be at least one descriptor
per statement, all with the same line number.  If column number is
also being represented they will have the column numbers of the start
of the statements also represented.
.P
There can also be more than one Dwarf_Line per statement.
For example, if a file is preprocessed by a language translator,
this could result in translator output showing 2 or more sets of line
numbers per translated line of output.

.H 3 "Get A Set of Lines"
The function returns information about every source line for a 
particular compilation-unit.  
The compilation-unit is specified
by the corresponding die.
.H 4 "dwarf_srclines()"
.DS
\f(CWint dwarf_srclines(
        Dwarf_Die die, 
        Dwarf_Line **linebuf, 
	Dwarf_Signed *linecount,
        Dwarf_Error *error)\fP
.DE
The function \f(CWdwarf_srclines()\fP places all line number descriptors 
for a single compilation unit into a single block, sets \f(CW*linebuf\fP 
to point to that block, 
sets \f(CW*linecount\fP to the number of descriptors in this block
and returns \f(CWDW_DLV_OK\fP.
The compilation-unit is indicated by the given \f(CWdie\fP which must be
a compilation-unit die.  
It returns \f(CWDW_DLV_ERROR\fP on error.  
On
successful return, each line number information structure pointed to by 
an entry in the block should be free'd using \f(CWdwarf_dealloc()\fP with 
the allocation type \f(CWDW_DLA_LINE\fP when no longer of interest.  Also 
the block of descriptors itself should be free'd using \f(CWdwarf_dealloc()\fP
with the allocation type \f(CWDW_DLA_LIST\fP when no longer of interest.
.P
.in +2
.DS
\f(CWDwarf_Signed cnt;
Dwarf_Line *linebuf;
int sres;

if ((sres = dwarf_srclines(somedie, &linebuf,&cnt, &error)) == DW_DLV_OK) {

        for (i = 0; i < cnt; ++i) {
                /* use linebuf[i] */
                dwarf_dealloc(dbg, linebuf[i], DW_DLA_LINE);
        }
        dwarf_dealloc(dbg, linebuf, DW_DLA_LIST);
}\fP
.DE
.in -2

.H 3 "Get the set of Source File Names"

The function returns the names of the source files that have contributed
to the compilation-unit represented by the given DIE.  Only the source
files named in the statement program prologue are returned.


.DS
\f(CWint dwarf_srcfiles(
        Dwarf_Die die,
        char ***srcfiles,
        Dwarf_Signed *srccount,
        Dwarf_Error *error)\fP
.DE
When it succeeds
\f(CWdwarf_srcfiles()\fP returns 
\f(CWDW_DLV_OK\fP
and
puts
the number of source
files named in the statement program prologue indicated by the given
\f(CWdie\fP
into \f(CW*srccount\fP.  
Source files defined in the statement program are ignored.
The given \f(CWdie\fP should have the tag \f(CWDW_TAG_compile_unit\fP.
The location pointed to by \f(CWsrcfiles\fP is set to point to a list
of pointers to null-terminated strings that name the source
files.  
On a successful return from this function, each of the
strings returned should be individually free'd using \f(CWdwarf_dealloc()\fP
with the allocation type \f(CWDW_DLA_STRING\fP when no longer of
interest.  
This should be followed by free-ing the list using
\f(CWdwarf_dealloc()\fP with the allocation type \f(CWDW_DLA_LIST\fP.
It returns \f(CWDW_DLV_ERROR\fP on error. 
It returns \f(CWDW_DLV_NO_ENTRY\fP
if there is no
corresponding statement program (i.e., if there is no line information).
.in +2
.DS
\f(CWDwarf_Signed cnt;
char **srcfiles;
int res;

if ((res = dwarf_srcfiles(somedie, &srcfiles,&cnt &error)) == DW_DLV_OK) {

        for (i = 0; i < cnt; ++i) {
                /* use srcfiles[i] */
                dwarf_dealloc(dbg, srcfiles[i], DW_DLA_STRING);
        }
        dwarf_dealloc(dbg, srcfiles, DW_DLA_LIST);
}\fP
.DE
.in -2
.H 3 "Get information about a Single Table Line"
The following functions can be used on the \f(CWDwarf_Line\fP descriptors
returned by \f(CWdwarf_srclines()\fP to obtain information about the
source lines.

.H 4 "dwarf_linebeginstatement()"
.DS
\f(CWint dwarf_linebeginstatement(
        Dwarf_Line line, 
	Dwarf_Bool *return_bool,
        Dwarf_Error *error)\fP
.DE
The function \f(CWdwarf_linebeginstatement()\fP returns 
\f(CWDW_DLV_OK\fP and sets \f(CW*return_bool\fP
to
\fInon-zero\fP 
(if \f(CWline\fP represents a line number entry that is marked as
beginning a statement).  
or
\fIzero\fP ((if \f(CWline\fP represents a line number entry
that is not marked as beginning a statement).
It returns \f(CWDW_DLV_ERROR\fP on error.
It never returns \f(CWDW_DLV_NO_ENTRY\fP.

.P
.H 4 "dwarf_lineendsequence()"
.DS
\f(CWint dwarf_lineendsequence(
	Dwarf_Line line,
	Dwarf_Bool *return_bool,
	Dwarf_Error *error)\fP
.DE
The function \f(CWdwarf_lineendsequence()\fP returns
\f(CWDW_DLV_OK\fP and sets \f(CW*return_bool\fP
\fInon-zero\fP
if \f(CWline\fP represents a line number entry that is marked as
ending a text sequence)
or
\fIzero\fP ((if \f(CWline\fP represents a line number entry
that is not marked as ending a text sequence).
It returns \f(CWDW_DLV_ERROR\fP on error.
It never returns \f(CWDW_DLV_NO_ENTRY\fP.

.P
.H 4 "dwarf_lineno()"
.DS
\f(CWint dwarf_lineno(
        Dwarf_Line       line, 
	Dwarf_Unsigned * returned_lineno,
        Dwarf_Error    * error)\fP
.DE
The function \f(CWdwarf_lineno()\fP returns 
\f(CWDW_DLV_OK\fP and sets \f(CW*return_lineno\fP to
the source statement line 
number corresponding to the descriptor \f(CWline\fP.  
It returns \f(CWDW_DLV_ERROR\fP on error.
It never returns \f(CWDW_DLV_NO_ENTRY\fP.

.P
.H 4 "dwarf_lineaddr()"
.DS
\f(CWint dwarf_lineaddr(
        Dwarf_Line   line, 
	Dwarf_Addr  *return_lineaddr,
        Dwarf_Error *error)\fP
.DE
The function \f(CWdwarf_lineaddr()\fP returns 
\f(CWDW_DLV_OK\fP and sets \f(CW*return_lineaddr\fP to
the address associated 
with the descriptor \f(CWline\fP.  
It returns \f(CWDW_DLV_ERROR\fP  on error.
It never returns \f(CWDW_DLV_NO_ENTRY\fP.

.P
.H 4 "dwarf_lineoff()"
.DS
\f(CWint dwarf_lineoff(
        Dwarf_Line line, 
	Dwarf_Signed   * return_lineoff,
        Dwarf_Error *error)\fP
.DE
The function \f(CWdwarf_lineoff()\fP returns 
\f(CWDW_DLV_OK\fP and sets \f(CW*return_lineoff\fP to
the column number at which
the statement represented by \f(CWline\fP begins.  
It sets \f(CWreturn_lineoff\fP to \fI-1\fP 
if the column number of the statement is not represented
(meaning the producer library call was given zero
as the column number). 
.P
On error it returns \f(CWDW_DLV_ERROR\fP.
It never returns \f(CWDW_DLV_NO_ENTRY\fP.

.H 4 "dwarf_linesrc()"
.DS
\f(CWint dwarf_linesrc(
        Dwarf_Line line, 
	char  **   return_linesrc,
        Dwarf_Error *error)\fP
.DE
The function \f(CWdwarf_linesrc()\fP returns
\f(CWDW_DLV_OK\fP and sets \f(CW*return_linesrc\fP to
a pointer to a
null-terminated string of characters that represents the name of the 
source-file where \f(CWline\fP occurs.  
It returns \f(CWDW_DLV_ERROR\fP on 
error.  
.P
If the applicable file name in the line table Statement Program Prolog 
does not start with a '/' character
the string in \f(CWDW_AT_comp_dir\fP (if applicable and present)
or the applicable
directory name from the line Statement Program Prolog 
is prepended to the
file name in the line table Statement Program Prolog
to make a full path.
.P
The storage pointed to by a successful return of 
\f(CWdwarf_linesrc()\fP should be free'd using \f(CWdwarf_dealloc()\fP with
the allocation type \f(CWDW_DLA_STRING\fP when no longer of interest.
It never returns \f(CWDW_DLV_NO_ENTRY\fP.

.H 4 "dwarf_lineblock()"
.DS
\f(CWint dwarf_lineblock(
        Dwarf_Line line, 
	Dwarf_Bool *return_bool,
        Dwarf_Error *error)\fP
.DE
The function
\f(CWdwarf_lineblock()\fP returns 
\f(CWDW_DLV_OK\fP and sets \f(CW*return_linesrc\fP to
non-zero (i.e. true)(if the line is marked as 
beginning a basic block)
or zero (i.e. false) (if the line is marked as not
beginning a basic block).  
It returns \f(CWDW_DLV_ERROR\fP on error.
It never returns \f(CWDW_DLV_NO_ENTRY\fP.

.\"#if 0
.\".H 3 "Finding a Line Given A PC value"
.\"This is a 'higher level' (High-level) interface to line information.
.\"
.\".DS
.\"\f(CWint dwarf_pclines(
.\"        Dwarf_Debug dbg,
.\"        Dwarf_Addr pc,
.\"        Dwarf_Line **linebuf,
.\"        Dwarf_Signed slide,
.\"        Dwarf_Signed *linecount,
.\"        Dwarf_Error *error)\fP
.\".DE
.\"The function \f(CWdwarf_pclines()\fP places all line number descriptors 
.\"that correspond to the value of \f(CWpc\fP into a single block and sets 
.\"\f(CWlinebuf\fP to point to that block.  A count of the number of 
.\"\f(CWDwarf_Line\fP descriptors that are in this block is returned.  For 
.\"most cases, the count returned will be \fIone\fP, though it may be higher 
.\"if optimizations such as common subexpression elimination result in multiple 
.\"line number entries for a given value of \f(CWpc\fP.  The \f(CWslide\fP 
.\"argument specifies the direction to search for the nearest line number 
.\"entry in the event that there is no line number entry that contains an 
.\"exact match for \f(CWpc\fP.  This argument may be one of: 
.\"\f(CWDLS_BACKWARD\fP, \f(CWDLS_NOSLIDE\fP, \f(CWDLS_FORWARD\fP.
.\"\f(CWDW_DLV_NOCOUNT\fP is returned on error.  On successful return, each 
.\"line information structure pointed to by an entry in the block should be 
.\"free'd using \f(CWdwarf_dealloc()\fP with the allocation type 
.\"\f(CWDW_DLA_LINE\fP when no longer of interest.  The block itself should 
.\"be free'd using \f(CWdwarf_dealloc()\fP with the allocation type 
.\"\f(CWDW_DLA_LIST\fP when no longer of interest.
.\"#endif

.H 2 "Global Name Space Operations" 
These operations operate on the .debug_pubnames section of the debugging 
information.

.H 3 "Debugger Interface Operations"

.H 4 "dwarf_get_globals()"
.DS
\f(CWint dwarf_get_globals(
        Dwarf_Debug dbg,
        Dwarf_Global **globals,
        Dwarf_Signed * return_count,
        Dwarf_Error *error)\fP
.DE
The function \f(CWdwarf_get_globals()\fP returns 
\f(CWDW_DLV_OK\fP and sets \f(CW*return_count\fP to
the count of pubnames
represented in the section containing pubnames i.e. .debug_pubnames.  
It also stores at \f(CW*globals\fP, a pointer 
to a list of \f(CWDwarf_Global\fP descriptors, one for each of the 
pubnames in the .debug_pubnames section.  
It returns \f(CWDW_DLV_ERROR\fP on error. 
It returns \f(CWDW_DLV_NO_ENTRY\fP if the .debug_pubnames 
section does not exist.

On a successful return from 
this function, the \f(CWDwarf_Global\fP descriptors should be individually 
free'd using \f(CWdwarf_dealloc()\fP with the allocation type 
\f(CWDW_DLA_GLOBAL\fP, followed by the deallocation of the list itself 
with the allocation type \f(CWDW_DLA_LIST\fP when the descriptors are 
no longer of interest.

.in +2
.DS
\f(CWDwarf_Signed cnt;
Dwarf_Global *globs;
int res;

if ((res = dwarf_get_globals(dbg, &globs,&cnt, &error)) == DW_DLV_OK) {

        for (i = 0; i < cnt; ++i) {
                /* use globs[i] */
                dwarf_dealloc(dbg, globs[i], DW_DLA_GLOBAL);
        }
        dwarf_dealloc(dbg, globs, DW_DLA_LIST);
}\fP
.DE
.in -2

.H 4 "dwarf_globname()"
.DS
\f(CWint dwarf_globname(
        Dwarf_Global global,
        char **      return_name,
        Dwarf_Error *error)\fP
.DE
The function \f(CWdwarf_globname()\fP returns
\f(CWDW_DLV_OK\fP and sets \f(CW*return_name\fP to
a pointer to a 
null-terminated string that names the pubname represented by the 
\f(CWDwarf_Global\fP descriptor, \f(CWglobal\fP.  
It returns \f(CWDW_DLV_ERROR\fP on error.  
On a successful return from this function, the string should
be free'd using \f(CWdwarf_dealloc()\fP, with the allocation type
\f(CWDW_DLA_STRING\fP when no longer of interest.
It never returns \f(CWDW_DLV_NO_ENTRY\fP.

.H 4 "dwarf_global_die_offset()"
.DS
\f(CWint dwarf_global_die_offset(
        Dwarf_Global global,
	Dwarf_Off   *return_offset,
        Dwarf_Error *error)\fP
.DE
The function \f(CWdwarf_global_die_offset()\fP returns
\f(CWDW_DLV_OK\fP and sets \f(CW*return_offset\fP to
the offset in
the section containing DIE's, i.e. .debug_info, of the DIE representing
the pubname that is described by the \f(CWDwarf_Global\fP descriptor, 
\f(CWglob\fP.  
It returns \f(CWDW_DLV_ERROR\fP on error.
It never returns \f(CWDW_DLV_NO_ENTRY\fP.

.H 4 "dwarf_global_cu_offset()"
.DS
\f(CWint dwarf_global_cu_offset(
        Dwarf_Global global,
	Dwarf_Off   *return_offset,
        Dwarf_Error *error)\fP
.DE
The function \f(CWdwarf_global_cu_offset()\fP returns 
\f(CWDW_DLV_OK\fP and sets \f(CW*return_offset\fP to
the offset in
the section containing DIE's, i.e. .debug_info, of the compilation-unit
header of the compilation-unit that contains the pubname described 
by the \f(CWDwarf_Global\fP descriptor, \f(CWglobal\fP.  
It returns \f(CWDW_DLV_ERROR\fP on error.
It never returns \f(CWDW_DLV_NO_ENTRY\fP.

.H 4 "dwarf_global_name_offsets()"
.DS
\f(CWint dwarf_global_name_offsets(
        Dwarf_Global global,
        char     **return_name,
        Dwarf_Off *die_offset,
        Dwarf_Off *cu_offset,
        Dwarf_Error *error)\fP
.DE
The function \f(CWdwarf_global_name_offsets()\fP returns
\f(CWDW_DLV_OK\fP and sets \f(CW*return_name\fP to
a pointer to
a null-terminated string that gives the name of the pubname
described by the \f(CWDwarf_Global\fP descriptor \f(CWglobal\fP.  
It returns \f(CWDW_DLV_ERROR\fP on error.  
It never returns \f(CWDW_DLV_NO_ENTRY\fP.
It also returns in the locations 
pointed to by \f(CWdie_offset\fP, and \f(CWcu_offset\fP, the offsets
of the DIE representing the 
pubname, and the DIE
representing the compilation-unit containing the 
pubname, respectively.
On a 
successful return from \f(CWdwarf_global_name_offsets()\fP the storage 
pointed to by \f(CWreturn_name\fP 
should be free'd using \f(CWdwarf_dealloc()\fP, 
with the allocation type \f(CWDW_DLA_STRING\fP when no longer of interest.

.H 2 "Weak Name Space Operations" 
These operations operate on the .debug_weaknames section of the debugging 
information.
.P
These operations are SGI specific, not part of standard DWARF.
.P

.H 3 "Debugger Interface Operations"

.H 4 "dwarf_get_weaks()"
.DS
\f(CWint dwarf_get_weaks(
        Dwarf_Debug dbg,
        Dwarf_Weak **weaks,
	Dwarf_Signed *weak_count,
        Dwarf_Error *error)\fP
.DE
The function \f(CWdwarf_get_weaks()\fP returns
\f(CWDW_DLV_OK\fP and sets \f(CW*weak_count\fP to
the count of weak names
represented in the section containing weak names i.e. .debug_weaknames.  
It returns \f(CWDW_DLV_ERROR\fP on error. 
It returns \f(CWDW_DLV_NO_ENTRY\fP if the section does not exist.  
It also stores in \f(CW*weaks\fP, a pointer to 
a list of \f(CWDwarf_Weak\fP descriptors, one for each of the weak names 
in the .debug_weaknames section.  
On a successful return from this function, 
the \f(CWDwarf_Weak\fP descriptors should be individually free'd using 
\f(CWdwarf_dealloc()\fP with the allocation type \f(CWDW_DLA_WEAK\fP, 
followed by the deallocation of the list itself with the allocation type 
\f(CWDW_DLA_LIST\fP when the descriptors are no longer of interest.

.in +2
.DS
\f(CWDwarf_Signed cnt;
Dwarf_Weak *weaks;
int res;

if ((res = dwarf_get_weaks(dbg, &weaks,&cnt, &error)) == DW_DLV_OK) {

        for (i = 0; i < cnt; ++i) {
                /* use weaks[i] */
                dwarf_dealloc(dbg, weaks[i], DW_DLA_WEAK);
        }
        dwarf_dealloc(dbg, weaks, DW_DLA_LIST);
}\fP
.DE
.in -2

.H 4 "dwarf_weakname()"
.DS
\f(CWint dwarf_weakname(
        Dwarf_Weak weak,
	char    ** return_name,
        Dwarf_Error *error)\fP
.DE
The function \f(CWdwarf_weakname()\fP returns
\f(CWDW_DLV_OK\fP and sets \f(CW*return_name\fP to
a pointer to a null-terminated
string that names the weak name represented by the 
\f(CWDwarf_Weak\fP descriptor, \f(CWweak\fP.  
It returns \f(CWDW_DLV_ERROR\fP on error.  
It never returns \f(CWDW_DLV_NO_ENTRY\fP.
On a successful return from this function, the string should
be free'd using \f(CWdwarf_dealloc()\fP, with the allocation type
\f(CWDW_DLA_STRING\fP when no longer of interest.

.DS
\f(CWint dwarf_weak_die_offset(
        Dwarf_Weak weak,
	Dwarf_Off  *return_offset,
        Dwarf_Error *error)\fP
.DE
The function \f(CWdwarf_weak_die_offset()\fP returns
\f(CWDW_DLV_OK\fP and sets \f(CW*return_offset\fP to the offset in
the section containing DIE's, i.e. .debug_info, of the DIE representing
the weak name that is described by the \f(CWDwarf_Weak\fP descriptor, 
\f(CWweak\fP.  
It returns \f(CWDW_DLV_ERROR\fP on error.
It never returns \f(CWDW_DLV_NO_ENTRY\fP.

.H 4 "dwarf_weak_cu_offset()"
.DS
\f(CWint dwarf_weak_cu_offset(
        Dwarf_Weak weak,
	Dwarf_Off  *return_offset,
        Dwarf_Error *error)\fP
.DE
The function \f(CWdwarf_weak_cu_offset()\fP returns
\f(CWDW_DLV_OK\fP and sets \f(CW*return_offset\fP to the offset in
the section containing DIE's, i.e. .debug_info, of the compilation-unit
header of the compilation-unit that contains the weak name described 
by the \f(CWDwarf_Weak\fP descriptor, \f(CWweak\fP.  
It returns \f(CWDW_DLV_ERROR\fP on error.
It never returns \f(CWDW_DLV_NO_ENTRY\fP.

.H 4 "dwarf_weak_name_offsets()"
.DS
\f(CWint dwarf_weak_name_offsets(
        Dwarf_Weak weak,
	char **  weak_name,
        Dwarf_Off *die_offset,
        Dwarf_Off *cu_offset,
        Dwarf_Error *error)\fP
.DE
The function \f(CWdwarf_weak_name_offsets()\fP returns
\f(CWDW_DLV_OK\fP and sets \f(CW*weak_name\fP to
a pointer to
a null-terminated string that gives the name of the weak name
described by the \f(CWDwarf_Weak\fP descriptor \f(CWweak\fP.  
It also returns in the locations 
pointed to by \f(CWdie_offset\fP, and \f(CWcu_offset\fP, the offsets
of the DIE representing the 
weakname, and the DIE
representing the compilation-unit containing the 
weakname, respectively.
It returns \f(CWDW_DLV_ERROR\fP on error.  
It never returns \f(CWDW_DLV_NO_ENTRY\fP.
On a 
successful return from \f(CWdwarf_weak_name_offsets()\fP the storage 
pointed to by \f(CWweak_name\fP
should be free'd using \f(CWdwarf_dealloc()\fP, 
with the allocation type \f(CWDW_DLA_STRING\fP when no longer of interest.

.H 2 "Static Function Names Operations"
This section is SGI specific and is not part of standard DWARF version 2.
.P
These function operate on the .debug_funcnames section of the debugging
information.  The .debug_funcnames section contains the names of static
functions defined in the object, the offsets of the \f(CWDIE\fPs that
represent the definitions of the corresponding functions, and the offsets
of the start of the compilation-units that contain the definitions of
those functions.

.H 3 "Debugger Interface Operations"

.H 4 "dwarf_get_funcs()"
.DS
\f(CWint dwarf_get_funcs(
        Dwarf_Debug dbg,
        Dwarf_Func **funcs,
        Dwarf_Signed *func_count,
        Dwarf_Error *error)\fP
.DE
The function \f(CWdwarf_get_funcs()\fP returns
\f(CWDW_DLV_OK\fP and sets \f(CW*func_count\fP to
the count of static
function names represented in the section containing static function
names, i.e. .debug_funcnames.  
It also 
stores, at \f(CW*funcs\fP, a pointer to a list of \f(CWDwarf_Func\fP 
descriptors, one for each of the static functions in the .debug_funcnames 
section.  
It returns \f(CWDW_DLV_NOCOUNT\fP on error.
It returns \f(CWDW_DLV_NO_ENTRY\fP
if the .debug_funcnames section does not exist.  
On a successful return from this function, the \f(CWDwarf_Func\fP 
descriptors should be individually free'd using \f(CWdwarf_dealloc()\fP 
with the allocation type \f(CWDW_DLA_FUNC\fP, followed by the deallocation 
of the list itself with the allocation type \f(CWDW_DLA_LIST\fP when 
the descriptors are no longer of interest.

.in +2
.DS
\f(CWDwarf_Signed cnt;
Dwarf_Func *funcs;
int fres;

if ((fres = dwarf_get_funcs(dbg, &funcs, &error)) == DW_DLV_OK) {

        for (i = 0; i < cnt; ++i) {
                /* use funcs[i] */
                dwarf_dealloc(dbg, funcs[i], DW_DLA_FUNC);
        }
        dwarf_dealloc(dbg, funcs, DW_DLA_LIST);
}\fP
.DE
.in -2

.H 4 "dwarf_funcname()"
.DS
\f(CWint dwarf_funcname(
        Dwarf_Func func,
        char **    return_name,
        Dwarf_Error *error)\fP
.DE
The function \f(CWdwarf_funcname()\fP returns
\f(CWDW_DLV_OK\fP and sets \f(CW*return_name\fP to
a pointer to a
null-terminated string that names the static function represented by the
\f(CWDwarf_Func\fP descriptor, \f(CWfunc\fP.  
It returns \f(CWDW_DLV_ERROR\fP on error.  
It never returns \f(CWDW_DLV_NO_ENTRY\fP.
On a successful return from this function, the string should
be free'd using \f(CWdwarf_dealloc()\fP, with the allocation type
\f(CWDW_DLA_STRING\fP when no longer of interest.

.H 4 "dwarf_func_die_offset()"
.DS
\f(CWint dwarf_func_die_offset(
        Dwarf_Func func,
	Dwarf_Off  *return_offset,
        Dwarf_Error *error)\fP
.DE
The function \f(CWdwarf_func_die_offset()\fP, returns
\f(CWDW_DLV_OK\fP and sets \f(CW*return_offset\fP to
the offset in
the section containing DIE's, i.e. .debug_info, of the DIE representing
the static function that is described by the \f(CWDwarf_Func\fP 
descriptor, \f(CWfunc\fP.  
It returns \f(CWDW_DLV_ERROR\fP on error.
It never returns \f(CWDW_DLV_NO_ENTRY\fP.

.H 4 "dwarf_func_cu_offset()"
.DS
\f(CWint dwarf_func_cu_offset(
        Dwarf_Func func,
	Dwarf_Off  *return_offset,
        Dwarf_Error *error)\fP
.DE
The function \f(CWdwarf_func_cu_offset()\fP returns
\f(CWDW_DLV_OK\fP and sets \f(CW*return_offset\fP to
the offset in
the section containing DIE's, i.e. .debug_info, of the compilation-unit
header of the compilation-unit that contains the static function
described by the \f(CWDwarf_Func\fP descriptor, \f(CWfunc\fP.  
It returns \f(CWDW_DLV_ERROR\fP on error.
It never returns \f(CWDW_DLV_NO_ENTRY\fP.

.H 4 "dwarf_func_name_offsets()"
.DS
\f(CWint dwarf_func_name_offsets(
        Dwarf_Func func,
	char     **func_name,
        Dwarf_Off *die_offset,
        Dwarf_Off *cu_offset,
        Dwarf_Error *error)\fP
.DE
The function \f(CWdwarf_func_name_offsets()\fP returns
\f(CWDW_DLV_OK\fP and sets \f(CW*func_name\fP to
a pointer to
a null-terminated string that gives the name of the static
function described by the \f(CWDwarf_Func\fP descriptor \f(CWfunc\fP.
It also returns in the locations
pointed to by \f(CWdie_offset\fP, and \f(CWcu_offset\fP, the offsets
of the DIE representing the 
static function, and the DIE
representing the compilation-unit containing the 
static function, respectively.
It returns \f(CWDW_DLV_ERROR\fP on error.  
It never returns \f(CWDW_DLV_NO_ENTRY\fP.
On a successful return from \f(CWdwarf_func_name_offsets()\fP
the storage pointed to by  \f(CWfunc_name\fP should be free'd using
\f(CWdwarf_dealloc()\fP, with the allocation type \f(CWDW_DLA_STRING\fP
when no longer of interest.

.H 2 "User Defined Type Names Operations"
This section is SGI specific and is not part of standard DWARF version 2.
.P
These functions operate on the .debug_typenames section of the debugging
information.  The .debug_typenames section contains the names of file-scope
user-defined types, the offsets of the \f(CWDIE\fPs that represent the
definitions of those types, and the offsets of the compilation-units 
that contain the definitions of those types.

.H 3 "Debugger Interface Operations"

.H 4 "dwarf_get_types()"
.DS
\f(CWint dwarf_get_types(
        Dwarf_Debug dbg,
        Dwarf_Type **types,
        Dwarf_Signed *typecount,
        Dwarf_Error *error)\fP
.DE
The function \f(CWdwarf_get_types()\fP returns
\f(CWDW_DLV_OK\fP and sets \f(CW*typecount\fP to
the count of user-defined
type names represented in the section containing user-defined type names,
i.e. .debug_typenames.  
It also stores at \f(CW*types\fP, 
a pointer to a list of \f(CWDwarf_Type\fP descriptors, one for each of the 
user-defined type names in the .debug_typenames section.  
It returns \f(CWDW_DLV_NOCOUNT\fP on error. 
It returns \f(CWDW_DLV_NO_ENTRY\fP if 
the .debug_typenames section does not exist.  
On a successful 
return from this function, the \f(CWDwarf_Type\fP descriptors should be 
individually free'd using \f(CWdwarf_dealloc()\fP with the allocation type 
\f(CWDW_DLA_TYPENAME\fP, followed by the deallocation of the list itself 
with the allocation type \f(CWDW_DLA_LIST\fP when the descriptors are no 
longer of interest.

.in +2
.DS
\f(CWDwarf_Signed cnt;
Dwarf_Type *types;
int res;

if ((res = dwarf_get_types(dbg, &types,&cnt, &error)) == DW_DLV_OK) {

        for (i = 0; i < cnt; ++i) {
                /* use types[i] */
                dwarf_dealloc(dbg, types[i], DW_DLA_TYPENAME);
        }
        dwarf_dealloc(dbg, types, DW_DLA_LIST);
}\fP
.DE
.in -2

.H 4 "dwarf_typename()"
.DS
\f(CWint dwarf_typename(
        Dwarf_Type   type,
        char       **return_name,
        Dwarf_Error *error)\fP
.DE
The function \f(CWdwarf_typename()\fP returns
\f(CWDW_DLV_OK\fP and sets \f(CW*return_name\fP to
a pointer to a
null-terminated string that names the user-defined type represented by the
\f(CWDwarf_Type\fP descriptor, \f(CWtype\fP.  
It returns \f(CWDW_DLV_ERROR\fP on error.  
It never returns \f(CWDW_DLV_NO_ENTRY\fP.
On a successful return from this function, the string should
be free'd using \f(CWdwarf_dealloc()\fP, with the allocation type
\f(CWDW_DLA_STRING\fP when no longer of interest.

.H 4 "dwarf_type_die_offset()"
.DS
\f(CWint dwarf_type_die_offset(
        Dwarf_Type type,
        Dwarf_Off  *return_offset,
        Dwarf_Error *error)\fP
.DE
The function \f(CWdwarf_type_die_offset()\fP returns
\f(CWDW_DLV_OK\fP and sets \f(CW*return_offset\fP to
the offset in
the section containing DIE's, i.e. .debug_info, of the DIE representing
the user-defined type that is described by the \f(CWDwarf_Type\fP 
descriptor, \f(CWtype\fP.  
It returns \f(CWDW_DLV_ERROR\fP on error.
It never returns \f(CWDW_DLV_NO_ENTRY\fP.

.H 4 "dwarf_type_cu_offset()"
.DS
\f(CWint dwarf_type_cu_offset(
        Dwarf_Type type,
        Dwarf_Off  *return_offset,
        Dwarf_Error *error)\fP
.DE
The function \f(CWdwarf_type_cu_offset()\fP returns
\f(CWDW_DLV_OK\fP and sets \f(CW*return_offset\fP to
the offset in
the section containing DIE's, i.e. .debug_info, of the compilation-unit
header of the compilation-unit that contains the user-defined type
described by the \f(CWDwarf_Type\fP descriptor, \f(CWtype\fP.  
It returns \f(CWDW_DLV_ERROR\fP on error.
It never returns \f(CWDW_DLV_NO_ENTRY\fP.

.H 4 "dwarf_type_name_offsets()"
.DS
\f(CWint dwarf_type_name_offsets(
        Dwarf_Type   type,
        char      ** returned_name,
        Dwarf_Off *  die_offset,
        Dwarf_Off *  cu_offset,
        Dwarf_Error *error)\fP
.DE
The function \f(CWdwarf_type_name_offsets()\fP returns
\f(CWDW_DLV_OK\fP and sets \f(CW*returned_name\fP to
a pointer to
a null-terminated string that gives the name of the user-defined
type described by the \f(CWDwarf_Type\fP descriptor \f(CWtype\fP.
It also returns in the locations
pointed to by \f(CWdie_offset\fP, and \f(CWcu_offset\fP, the offsets
of the DIE representing the 
user-defined type, and the DIE
representing the compilation-unit containing the 
user-defined type, respectively.
It returns \f(CWDW_DLV_ERROR\fP on error.  
It never returns \f(CWDW_DLV_NO_ENTRY\fP.
On a successful return from \f(CWdwarf_type_name_offsets()\fP
the storage pointed to by \f(CWreturned_name\fP should
be free'd using
\f(CWdwarf_dealloc()\fP, with the allocation type \f(CWDW_DLA_STRING\fP
when no longer of interest.


.H 2 "User Defined Static Variable Names Operations"
This section is SGI specific and is not part of standard DWARF version 2.
.P
These functions operate on the .debug_varnames section of the debugging
information.  The .debug_varnames section contains the names of file-scope
static variables, the offsets of the \f(CWDIE\fPs that represent the 
definitions of those variables, and the offsets of the compilation-units
that contain the definitions of those variables.
.P

.H 3 "Debugger Interface Operations"

.H 4 "dwarf_get_vars()"

.DS
\f(CWint dwarf_get_vars(
        Dwarf_Debug dbg,
        Dwarf_Var **vars,
        Dwarf_Signed *var_count,
        Dwarf_Error *error)\fP
.DE
The function \f(CWdwarf_get_vars()\fP returns
\f(CWDW_DLV_OK\fP and sets \f(CW*var_count\fP to
the count of file-scope
static variable names represented in the section containing file-scope 
static variable names, i.e. .debug_varnames.  
It also stores, at \f(CW*vars\fP, a pointer to a list of 
\f(CWDwarf_Var\fP descriptors, one for each of the file-scope static
variable names in the .debug_varnames section.  
It returns \f(CWDW_DLV_ERROR\fP on error.
It returns \f(CWDW_DLV_NO_ENTRY\fP if the .debug_varnames section does 
not exist.  
On a successful return 
from this function, the \f(CWDwarf_Var\fP descriptors should be individually 
free'd using \f(CWdwarf_dealloc()\fP with the allocation type 
\f(CWDW_DLA_VAR\fP, followed by the deallocation of the list itself with 
the allocation type \f(CWDW_DLA_LIST\fP when the descriptors are no 
longer of interest.

.in +2
.DS
\f(CWDwarf_Signed cnt;
Dwarf_Var *vars;
int res;

if ((res = dwarf_get_vars(dbg, &vars,&cnt &error)) == DW_DLV_OK) {

        for (i = 0; i < cnt; ++i) {
                /* use vars[i] */
                dwarf_dealloc(dbg, vars[i], DW_DLA_VAR);
        }
        dwarf_dealloc(dbg, vars, DW_DLA_LIST);
}\fP
.DE
.in -2

.H 4 "dwarf_varname()"
.DS
\f(CWint dwarf_varname(
        Dwarf_Var var,
        char **    returned_name,
        Dwarf_Error *error)\fP
.DE
The function \f(CWdwarf_varname()\fP returns
\f(CWDW_DLV_OK\fP and sets \f(CW*returned_name\fP to
a pointer to a
null-terminated string that names the file-scope static variable represented 
by the \f(CWDwarf_Var\fP descriptor, \f(CWvar\fP.  
It returns \f(CWDW_DLV_ERROR\fP on error.  
It never returns \f(CWDW_DLV_NO_ENTRY\fP.
On a successful return from this function, the string should
be free'd using \f(CWdwarf_dealloc()\fP, with the allocation type
\f(CWDW_DLA_STRING\fP when no longer of interest.

.H 4 "dwarf_var_die_offset()"
.DS
\f(CWint dwarf_var_die_offset(
        Dwarf_Var    var,
        Dwarf_Off   *returned_offset,
        Dwarf_Error *error)\fP
.DE
The function \f(CWdwarf_var_die_offset()\fP returns
\f(CWDW_DLV_OK\fP and sets \f(CW*returned_offset\fP to
the offset in
the section containing DIE's, i.e. .debug_info, of the DIE representing
the file-scope static variable that is described by the \f(CWDwarf_Var\fP 
descriptor, \f(CWvar\fP.  
It returns \f(CWDW_DLV_ERROR\fP on error.
It never returns \f(CWDW_DLV_NO_ENTRY\fP.

.H 4 "dwarf_var_cu_offset()"
.DS
\f(CWint dwarf_var_cu_offset(
        Dwarf_Var var,
        Dwarf_Off   *returned_offset,
        Dwarf_Error *error)\fP
.DE
The function \f(CWdwarf_var_cu_offset()\fP returns
\f(CWDW_DLV_OK\fP and sets \f(CW*returned_offset\fP to
the offset in
the section containing DIE's, i.e. .debug_info, of the compilation-unit
header of the compilation-unit that contains the file-scope static
variable described by the \f(CWDwarf_Var\fP descriptor, \f(CWvar\fP.  
It returns \f(CWDW_DLV_ERROR\fP on error.
It never returns \f(CWDW_DLV_NO_ENTRY\fP.

.H 4 "dwarf_var_name_offsets()"
.DS
\f(CWint dwarf_var_name_offsets(
        Dwarf_Var var,
	char     **returned_name,
        Dwarf_Off *die_offset,
        Dwarf_Off *cu_offset,
        Dwarf_Error *error)\fP
.DE
The function \f(CWdwarf_var_name_offsets()\fP returns
\f(CWDW_DLV_OK\fP and sets \f(CW*returned_name\fP to
a pointer to
a null-terminated string that gives the name of the file-scope
static variable described by the \f(CWDwarf_Var\fP descriptor \f(CWvar\fP.
It also returns in the locations
pointed to by \f(CWdie_offset\fP, and \f(CWcu_offset\fP, the offsets
of the DIE representing the 
file-scope static variable, and the DIE
representing the compilation-unit containing the 
file-scope static variable, respectively.
It returns \f(CWDW_DLV_ERROR\fP on error.  
It never returns \f(CWDW_DLV_NO_ENTRY\fP.
On a successful return from 
\f(CWdwarf_var_name_offsets()\fP the storage pointed to by
\f(CWreturned_name\fP
should be free'd using \f(CWdwarf_dealloc()\fP, with the allocation 
type \f(CWDW_DLA_STRING\fP when no longer of interest.

.H 2 "Macro Information Operations"
.H 3 "General Macro Operations"
.H 4 "dwarf_find_macro_value_start()"
.DS
\f(CWchar *dwarf_find_macro_value_start(char * macro_string);\fP
.DE
Given a macro string in the standard form defined in the DWARF
document ("name <space> value" or "name(args)<space>value")
this returns a pointer to the first byte of the macro value.
It does not alter the string pointed to by macro_string or copy
the string: it returns a pointer into the string whose
address was passed in.
.H 3 "Debugger Interface Macro Operations"
Macro information is accessed from the .debug_info section via the
DW_AT_macro_info attribute (whose value is an offset into .debug_macinfo).
.P
No Functions yet defined.
.H 3 "Low Level Macro Information Operations"
.H 4 "dwarf_get_macro_details()"
.DS
\f(CWint dwarf_get_macro_details(Dwarf_Debug /*dbg*/,
  Dwarf_Off              macro_offset,
  Dwarf_Unsigned         maximum_count,
  Dwarf_Signed         * entry_count,
  Dwarf_Macro_Details ** details,
  Dwarf_Error *          err);\fP
.DE
\f(CWdwarf_get_macro_details()\fP
returns  
\f(CWDW_DLV_OK\fP and sets
\f(CWentry_count\fP to the number of \f(CWdetails\fP records
returned through the \f(CWdetails\fP pointer.
The data returned thru  \f(CWdetails\fP should be freed
by a call to \f(CWdwarf_dealloc()\fP with the allocation type
\f(CWDW_DLA_STRING\fP.
If \f(CWDW_DLV_OK\fP is returned, the \f(CWentry_count\fP will
be at least 1, since
a compilation unit with macro information but no macros will
have at least one macro data byte of 0.
.P
\f(CWdwarf_get_macro_details()\fP
begins at the \f(CWmacro_offset\fP offset you supply
and ends at the end of a compilation unit or at \f(CWmaximum_count\fP
detail records (whichever comes first).
If \f(CWmaximum_count\fP is 0, it is treated as if it were the maximum
possible unsigned integer.
.P
\f(CWdwarf_get_macro_details()\fP
attempts to set \f(CWdmd_fileindex\fP to the correct file in every
\f(CWdetails\fP record. If it is unable to do so (or whenever
the current file index is unknown, it sets \f(CWdmd_fileindex\fP
to -1.
.P
\f(CWdwarf_get_macro_details()\fP returns \f(CWDW_DLV_ERROR\fP on error.
It returns \f(CWDW_DLV_NO_ENTRY\fP if there is no more
macro information at that \f(CWmacro_offset\fP. If \f(CWmacro_offset\fP
is passed in as 0, a \f(CWDW_DLV_NO_ENTRY\fP return means there is
no macro information.
.P
.in +2
.DS
\f(CWDwarf_Unsigned max = 0;
Dwarf_Off cur_off = 0;
Dwarf_Signed count = 0;
Dwarf_Macro_Details *maclist;
int errv;

/* loop thru all the compilation units macro info */
while((errv = dwarf_macro_details(dbg, cur_off,max,
     &count,&maclist,&error))== DW_DLV_OK) {
    for (i = 0; i < count; ++i) {
      /* use maclist[i] */
    }
    cur_off = maclist[count-1].dmd_offset + 1;
    dwarf_dealloc(dbg, maclist, DW_DLA_STRING);
}\fP
.DE


.H 2 "Low Level Frame Operations"
These functions provide information about stack frames to be
used to perform stack traces.  The information is an abstraction
of a table with a row per instruction and a column per register
and a column for the canonical frame address (CFA, which corresponds
to the notion of a frame pointer), 
as well as a column for the return address.  
Each cell in the table contains one of the following:

.AL 
.LI
A register + offset(a)(b)

.LI
A register(c)(d)

.LI
A marker (DW_FRAME_UNDEFINED_VAL) meaning \fIregister value undefined\fP

.LI
A marker (DW_FRAME_SAME_VAL) meaning \fIregister value same as in caller\fP
.LE
.P
(a) When  the column is DW_FRAME_CFA_COL: the register
number is a real hardware register, not a reference
to DW_FRAME_CFA_COL, not  DW_FRAME_UNDEFINED_VAL,
and not DW_FRAME_SAME_VAL. 
The CFA rule value should be the stack pointer
plus offset 0 when no other value makes sense.
A value of DW_FRAME_SAME_VAL would
be semi-logical, but since the CFA is not a real register,
not really correct.
A value of DW_FRAME_UNDEFINED_VAL would imply
the CFA is undefined  --
this seems to be a useless notion, as
the CFA is a means to finding real registers,
so those real registers should be marked DW_FRAME_UNDEFINED_VAL,
and the CFA column content (whatever register it
specifies) becomes unreferenced by anything.
.P
(b) When the column is not DW_FRAME_CFA_COL, the 'register'
will and must be DW_FRAME_CFA_COL, implying that
to get the final location for the column one must add
the offset here plus the DW_FRAME_CFA_COL rule value.
.P
(c) When the column is DW_FRAME_CFA_COL, then the register
number is (must be) a real hardware register .
If it were DW_FRAME_UNDEFINED_VAL or DW_FRAME_SAME_VAL
it would be a marker, not a register number.
.P
(d) When the column is not DW_FRAME_CFA_COL, the register
may be a hardware register.
It will not be DW_FRAME_CFA_COL.
.P
There is no 'column' for DW_FRAME_UNDEFINED_VAL or DW_FRAME_SAME_VAL.


Figure \n(aX
is machine dependent and represents MIPS cpu register
assignments.

.DS
.TS
center box, tab(:);
lfB lfB lfB
l c l.
NAME:value:PURPOSE
_
DW_FRAME_CFA_COL:0:column used for CFA
DW_FRAME_REG1:1:integer regster 1
DW_FRAME_REG2:2:integer register 2
---::obvious names and values here
DW_FRAME_REG30:30:integer register 30 
DW_FRAME_REG31:31:integer register 31
DW_FRAME_FREG0:32:floating point register 0
DW_FRAME_FREG1:33:floating point register 1
---::obvious names and values here
DW_FRAME_FREG30:62:floating point register 30
DW_FRAME_FREG31:63:floating point register 31
DW_FRAME_RA_COL:64:column recording ra
DW_FRAME_UNDEFINED_VAL:1034:register val undefined
DW_FRAME_SAME_VAL:1035:register same as in caller
.TE

.FG "Frame Information Rule Assignments"
.DE

.P
The following table shows SGI/MIPS specific
special cell values: these values mean 
that the cell has the value \fIundefined\fP or \fIsame value\fP
respectively, rather than containing a \fIregister\fP or
\fIregister+offset\fP.  
.P
.DS
.TS
center box, tab(:);
lfB lfB lfB
l c l.
NAME:value:PURPOSE
_
DW_FRAME_UNDEFINED_VAL:1034:means undefined value.
::Not a column or register value
DW_FRAME_SAME_VAL:1035:means 'same value' as
::caller had. Not a column or 
::register value
.TE
.FG "Frame Information Special Values"
.DE

.\"#if 0
.\".P
.\"Since the cie and fde entries are not "organized" by anything
.\"outside of the .debug_frame section one must scan them all to 
.\"find all entry addresses and lengths.  Since there is one fde 
.\"per function this can be a rather long list.
.\"#endif

.P
.H 4 "dwarf_get_fde_list()"
.DS
\f(CWint dwarf_get_fde_list(
        Dwarf_Debug dbg,
        Dwarf_Cie **cie_data,
        Dwarf_Signed *cie_element_count,
        Dwarf_Fde **fde_data,
        Dwarf_Signed *fde_element_count,
        Dwarf_Error *error);\fP
.DE
\f(CWdwarf_get_fde_list()\fP stores a pointer to a list of 
\f(CWDwarf_Cie\fP descriptors in \f(CW*cie_data\fP, and the 
count of the number of descriptors in \f(CW*cie_element_count\fP.  
There is a descriptor for each CIE in the .debug_frame section.  
Similarly, it stores a pointer to a list of \f(CWDwarf_Fde\fP 
descriptors in \f(CW*fde_data\fP, and the count of the number 
of descriptors in \f(CW*fde_element_count\fP.  There is one 
descriptor per FDE in the .debug_frame section.  
\f(CWdwarf_get_fde_list()\fP  returns \f(CWDW_DLV_EROR\fP on error.
It returns \f(CWDW_DLV_NO_ENTRY\fP if it cannot find frame entries.
It returns \f(CWDW_DLV_OK\fP on a successful return.

On successful return, each of the structures pointed to by a 
descriptor should be individually free'd using \f(CWdwarf_dealloc()\fP 
with either the allocation type \f(CWDW_DLA_CIE\fP, or \f(CWDW_DLA_FDE\fP 
as appropriate when no longer of interest.  Each of the blocks 
of descriptors should be free'd using \f(CWdwarf_dealloc()\fP with 
the allocation type \f(CWDW_DLA_LIST\fP when no longer of interest.


.in +2
.DS
\f(CWDwarf_Signed cnt;
Dwarf_Cie *cie_data;
Dwarf_Signed cie_count;
Dwarf_Fde *fde_data;
Dwarf_Signed fde_count;
int fres;

if ((fres = dwarf_get_fde_list(dbg,&cie_data,&cie_count, 
		&fde_data,&fde_count,&error)) == DW_DLV_OK) {

        for (i = 0; i < cie_count; ++i) {
                /* use cie[i] */
                dwarf_dealloc(dbg, cie_data[i], DW_DLA_CIE);
        }
        for (i = 0; i < fde_count; ++i) {
                /* use fde[i] */
                dwarf_dealloc(dbg, fde_data[i], DW_DLA_FDE);
        }
        dwarf_dealloc(dbg, cie_data, DW_DLA_LIST);
        dwarf_dealloc(dbg, fde_data, DW_DLA_LIST);
}\fP
.DE
.in -2

.P
.H 4 "dwarf_get_fde_list_eh()"
.DS
\f(CWint dwarf_get_fde_list_eh(
        Dwarf_Debug dbg,
        Dwarf_Cie **cie_data,
        Dwarf_Signed *cie_element_count,
        Dwarf_Fde **fde_data,
        Dwarf_Signed *fde_element_count,
        Dwarf_Error *error);\fP
.DE
\f(CWdwarf_get_fde_list_eh()\fP is identical to
\f(CWdwarf_get_fde_list()\fP except that
\f(CWdwarf_get_fde_list_eh()\fP reads the GNU ecgs
section named .eh_frame (C++ exception handling information).

\f(CWdwarf_get_fde_list_eh()\fP stores a pointer to a list of
\f(CWDwarf_Cie\fP descriptors in \f(CW*cie_data\fP, and the
count of the number of descriptors in \f(CW*cie_element_count\fP.
There is a descriptor for each CIE in the .debug_frame section.
Similarly, it stores a pointer to a list of \f(CWDwarf_Fde\fP
descriptors in \f(CW*fde_data\fP, and the count of the number
of descriptors in \f(CW*fde_element_count\fP.  There is one
descriptor per FDE in the .debug_frame section.
\f(CWdwarf_get_fde_list()\fP  returns \f(CWDW_DLV_EROR\fP on error.
It returns \f(CWDW_DLV_NO_ENTRY\fP if it cannot find 
exception handling entries.
It returns \f(CWDW_DLV_OK\fP on a successful return.

On successful return, each of the structures pointed to by a
descriptor should be individually free'd using \f(CWdwarf_dealloc()\fP
with either the allocation type \f(CWDW_DLA_CIE\fP, or \f(CWDW_DLA_FDE\fP
as appropriate when no longer of interest.  Each of the blocks
of descriptors should be free'd using \f(CWdwarf_dealloc()\fP with
the allocation type \f(CWDW_DLA_LIST\fP when no longer of interest.


.P
.H 4 "dwarf_get_cie_of_fde()"
.DS
\f(CWint dwarf_get_cie_of_fde(Dwarf_Fde fde,
        Dwarf_Cie *cie_returned,
        Dwarf_Error *error);\fP
.DE
\f(CWdwarf_get_cie_of_fde()\fP stores a \f(CWDwarf_Cie\fP
into the  \f(CWDwarf_Cie\fP that \f(CWcie_returned\fP points at.

If one has called dwarf_get_fde_list and does not wish
to dwarf_dealloc() all the individual FDEs immediately, one
must also avoid dwarf_dealloc-ing the CIEs for those FDEs
not immediately dealloc'd.
Failing to observe this restriction will cause the  FDE(s) not
dealloced to become invalid: an FDE contains (hidden in it)
a CIE pointer which will be be invalid (stale, pointing to freed memory)
if the CIE is dealloc'd.
The invalid CIE pointer internal to the FDE cannot be detected
as invalid by libdwarf.
If one later passes an FDE with a stale internal CIE pointer
to one of the routines taking an FDE as input the result will
be failure of the call (returning DW_DLV_ERROR) at best and
it is possible a coredump or worse will happpen (eventually).


\f(CWdwarf_get_cie_of_fde()\fP returns 
\f(CWDW_DLV_OK\fP if it is successful (it will be
unless fde is the NULL pointer).
It returns \f(CWDW_DLV_ERROR\fP if the fde is invalid (NULL).

.P
Each \f(CWDwarf_Fde\fP descriptor describes information about the
frame for a particular subroutine or function.

\f(CWint dwarf_get_fde_for_die\fP is SGI/MIPS specific.

.H 4 "dwarf_get_fde_for_die()"
.DS
\f(CWint dwarf_get_fde_for_die(
        Dwarf_Debug dbg,
        Dwarf_Die die,
        Dwarf_Fde *  return_fde,
        Dwarf_Error *error)\fP
.DE
When it succeeds,
\f(CWdwarf_get_fde_for_die()\fP returns 
\f(CWDW_DLV_OK\fP and sets \f(CW*return_fde\fP
to
a \f(CWDwarf_Fde\fP
descriptor representing frame information for the given \f(CWdie\fP.  It 
looks for the \f(CWDW_AT_MIPS_fde\fP attribute in the given \f(CWdie\fP.
If it finds it, is uses the value of the attribute as the offset in 
the .debug_frame section where the FDE begins.
If there is no \f(CWDW_AT_MIPS_fde\fP it returns \f(CWDW_DLV_NO_ENTRY\fP.
If there is an error it returns \f(CWDW_DLV_ERROR\fP.

.H 4 "dwarf_get_fde_range()"
.DS
\f(CWint dwarf_get_fde_range(
        Dwarf_Fde fde,
        Dwarf_Addr *low_pc,
        Dwarf_Unsigned *func_length,
        Dwarf_Ptr *fde_bytes,
        Dwarf_Unsigned *fde_byte_length,
        Dwarf_Off *cie_offset,
        Dwarf_Signed *cie_index,
        Dwarf_Off *fde_offset,
        Dwarf_Error *error);\fP
.DE
On success,
\f(CWdwarf_get_fde_range()\fP returns
\f(CWDW_DLV_OK\fP.
The location pointed to by \f(CWlow_pc\fP is set to the low pc value for
this function.  
The location pointed to by \f(CWfunc_length\fP is 
set to the length of the function in bytes.  
This is essentially the
length of the text section for the function.  
The location pointed
to by \f(CWfde_bytes\fP is set to the address where the FDE begins
in the .debug_frame section.  
The location pointed to by 
\f(CWfde_byte_length\fP is set to the length in bytes of the portion
of .debug_frame for this FDE.  
This is the same as the value returned
by \f(CWdwarf_get_fde_range\fP.  
The location pointed to by 
\f(CWcie_offset\fP is set to the offset in the .debug_frame section
of the CIE used by this FDE.  
The location pointed to by \f(CWcie_index\fP
is set to the index of the CIE used by this FDE.  
The index is the 
index of the CIE in the list pointed to by \f(CWcie_data\fP as set 
by the function \f(CWdwarf_get_fde_list()\fP.  
However, if the function
\f(CWdwarf_get_fde_for_die()\fP was used to obtain the given \f(CWfde\fP, 
this index may not be correct.   
The location pointed to by 
\f(CWfde_offset\fP is set to the offset of the start of this FDE in 
the .debug_frame section.  
\f(CWdwarf_get_fde_range()\fP returns \f(CWDW_DLV_ERROR\fP on error.

.H 4 "dwarf_get_cie_info()"
.DS
\f(CWint dwarf_get_cie_info(
        Dwarf_Cie       cie,
	Dwarf_Unsigned *bytes_in_cie,
        Dwarf_Small    *version,
        char          **augmenter,
        Dwarf_Unsigned *code_alignment_factor,
        Dwarf_Signed *data_alignment_factor,
        Dwarf_Half     *return_address_register_rule,
        Dwarf_Ptr      *initial_instructions,
        Dwarf_Unsigned *initial_instructions_length,
        Dwarf_Error    *error);\fP
.DE
\f(CWdwarf_get_cie_info()\fP is primarily for Internal-level Interface 
consumers.  
If successful,
it returns
\f(CWDW_DLV_OK\fP and sets \f(CW*bytes_in_cie\fP to
the number of bytes in the portion of the
frames section for the CIE represented by the given \f(CWDwarf_Cie\fP
descriptor, \f(CWcie\fP.  
The other fields are directly taken from 
the cie and returned, via the pointers to the caller.  
It returns \f(CWDW_DLV_ERROR\fP on error.

.H 4 "dwarf_get_fde_instr_bytes()"
.DS
\f(CWint dwarf_get_fde_instr_bytes(
        Dwarf_Fde fde,
	Dwarf_Ptr *outinstrs,
	Dwarf_Unsigned *outlen,
        Dwarf_Error *error);\fP
.DE
\f(CWdwarf_get_fde_instr_bytes()\fP returns
\f(CWDW_DLV_OK\fP and sets \f(CW*outinstrs\fP to
a pointer to a set of bytes which are the
actual frame instructions for this fde.
It also sets \f(CW*outlen\fP to the length, in
bytes, of the frame instructions.
It returns \f(CWDW_DLV_ERROR\fP on error.
It never returns \f(CWDW_DLV_NO_ENTRY\fP.
The intent is to allow low-level consumers like a dwarf-dumper
to print the bytes in some fashion.
The memory pointed to by \f(CWoutinstrs\fP
must not be changed and there
is nothing to free.

.H 4 "dwarf_get_fde_info_for_reg()"
.DS
\f(CWint dwarf_get_fde_info_for_reg(
        Dwarf_Fde fde,
        Dwarf_Half table_column,
        Dwarf_Addr pc_requested,
	Dwarf_Signed *offset_relevant,
        Dwarf_Signed *register_num,
        Dwarf_Signed *offset,
        Dwarf_Addr *row_pc,
        Dwarf_Error *error);\fP
.DE
\f(CWdwarf_get_fde_info_for_reg()\fP returns
\f(CWDW_DLV_OK\fP and sets \f(CW*offset_relevant\fP to
non-zero if the offset is relevant for the
row specified by \f(CWpc_requested\fP and column specified by
\f(CWtable_column\fP, for the FDE specified by \f(CWfde\fP.  
The
intent is to return the rule for the given pc value and register.
The location pointed to by \f(CWregister_num\fP is set to the register
value for the rule.  
The location pointed to by \f(CWoffset\fP 
is set to the offset value for the rule.  
If offset is not relevant for this rule, \f(CW*offset_relevant\fP is
set to zero.
Since more than one pc 
value will have rows with identical entries, the user may want to
know the earliest pc value after which the rules for all the columns
remained unchanged.  
Recall that in the virtual table that the frame information
represents there may be one or more table rows with identical data
(each such table row at a different pc value).
Given a \f(CWpc_requested\fP which refers to a pc in such a group
of identical rows, 
the location pointed to by \f(CWrow_pc\fP is set 
to the lowest pc value
within the group of  identical rows.
The  value put in \f(CW*register_num\fP any of the
\f(CWDW_FRAME_*\fP table columns values specified in \f(CWlibdwarf.h\fP
or \f(CWdwarf.h\fP.

\f(CWdwarf_get_fde_info_for_reg\fP returns \f(CWDW_DLV_ERROR\fP if there is an error. 

It is usable with either 
\f(CWdwarf_get_fde_n()\fP or \f(CWdwarf_get_fde_at_pc()\fP.

.H 4 "dwarf_get_fde_info_for_all_regs()"
.DS
\f(CWint dwarf_get_fde_info_for_all_regs(
        Dwarf_Fde fde,
        Dwarf_Addr pc_requested,
	Dwarf_Regtable *reg_table,
        Dwarf_Addr *row_pc,
        Dwarf_Error *error);\fP
.DE
\f(CWdwarf_get_fde_info_for_all_regs()\fP returns
\f(CWDW_DLV_OK\fP and sets \f(CW*reg_table\fP for the row specified by
\f(CWpc_requested\fP for the FDE specified by \f(CWfde\fP. The intent is
to return the rules for decoding all the registers, given a pc value.
\f(CWreg_table\fP is an array of rules, one for each register specified in
\f(CWdwarf.h\fP. The rule for each register contains three items - 
\f(CWdw_regnum\fP which denotes the register value for that rule,
\f(CWdw_offset\fP which denotes the offset value for that rule and 
\f(CWdw_offset_relevant\fP which is set to zero if offset is not relevant 
for that rule. See \f(CWdwarf_get_fde_info_fo_reg()\fP for a description 
of \f(CWrow_pc\fP.

\f(CWdwarf_get_fde_info_for_all_regs\fP returns \f(CWDW_DLV_ERROR\fP if there is an error. 

\f(CWint dwarf_get_fde_info_for_all_regs\fP is SGI/MIPS specific.
.H 4 "dwarf_get_fde_n()"
.DS
\f(CWint   dwarf_get_fde_n(
        Dwarf_Fde *fde_data,
        Dwarf_Unsigned fde_index,
	Dwarf_Fde      *returned_fde
        Dwarf_Error *error);\fP
.DE
\f(CWdwarf_get_fde_n()\fP returns
\f(CWDW_DLV_OK\fP and sets \f(CWreturned_fde\fP to
the \f(CWDwarf_Fde\fP descriptor whose 
index is \f(CWfde_index\fP in the table of \f(CWDwarf_Fde\fP descriptors
pointed to by \fPfde_data\fP.  
The index starts with 0.  
Returns \f(CWDW_DLV_NO_ENTRY\fP if the index does not 
exist in the table of \f(CWDwarf_Fde\fP 
descriptors. 
Returns \f(CWDW_DLV_ERROR\fP if there is an error.
This function cannot be used unless
the block of \f(CWDwarf_Fde\fP descriptors has been created by a call to
\f(CWdwarf_get_fde_list()\fP.

.H 4 "dwarf_get_fde_at_pc()"
.DS
\f(CWint   dwarf_get_fde_at_pc(
        Dwarf_Fde *fde_data,
        Dwarf_Addr pc_of_interest,
        Dwarf_Fde *returned_fde,
        Dwarf_Addr *lopc,
        Dwarf_Addr *hipc,
        Dwarf_Error *error);\fP
.DE
\f(CWdwarf_get_fde_at_pc()\fP returns
\f(CWDW_DLV_OK\fP and sets \f(CWreturned_fde\fP to
a \f(CWDwarf_Fde\fP descriptor
for a function which contains the pc value specified by \f(CWpc_of_interest\fP.
In addition, it sets the locations pointed to 
by \f(CWlopc\fP and \f(CWhipc\fP to the low address and the high address 
covered by this FDE, respectively.  
It returns \f(CWDW_DLV_ERROR\fP on error.
It returns \f(CWDW_DLV_NO_ENTRY\fP 
if \f(CWpc_of_interest\fP is not in any of the
FDEs represented by the block of \f(CWDwarf_Fde\fP descriptors pointed
to by \f(CWfde_data\fP.  
This function cannot be used unless
the block of \f(CWDwarf_Fde\fP descriptors has been created by a call to
\f(CWdwarf_get_fde_list()\fP.

.H 4 "dwarf_expand_frame_instructions()"
.DS
\f(CWint dwarf_expand_frame_instructions(
        Dwarf_Debug dbg,
        Dwarf_Ptr instruction,
        Dwarf_Unsigned i_length,
        Dwarf_Frame_Op **returned_op_list,
        Dwarf_Signed   * returned_op_count,
        Dwarf_Error *error);\fP
.DE
\f(CWdwarf_expand_frame_instructions()\fP is a High-level interface 
function which expands a frame instruction byte stream into an 
array of \f(CWDwarf_Frame_Op\fP structures.  
To indicate success, it returns \f(CWDW_DLV_OK\fP.
The address where 
the byte stream begins is specified by \f(CWinstruction\fP, and
the length of the byte stream is specified by \f(CWi_length\fP.
The location pointed to by \f(CWreturned_op_list\fP is set to
point to a table of 
\f(CWreturned_op_count\fP
pointers to \f(CWDwarf_Frame_Op\fP which
contain the frame instructions in the byte stream.  
It returns \f(CWDW_DLV_ERROR\fP on error. 
It never returns \f(CWDW_DLV_NO_ENTRY\fP.
After a successful return, the
array of structures should be freed using
\f(CWdwarf_dealloc()\fP with the allocation type \f(CWDW_DLA_FRAME_BLOCK\fP
(when they are no longer of interest).

.in +2
.DS
\f(CWDwarf_Signed cnt;
Dwarf_Frame_Op *frameops;
Dwarf_Ptr instruction;
Dwarf_Unsigned len;
int res;

if (expand_frame_instructions(dbg,instruction,len, &frameops,&cnt, &error)
         == DW_DLV_OK) {

        for (i = 0; i < cnt; ++i) {
                /* use frameops[i] */
        }
        dwarf_dealloc(dbg, frameops, DW_DLA_FRAME_BLOCK);
}\fP
.DE
.in -2


.H 2 "Location Expression Evaluation"

An "interpreter" which evaluates a location expression
is required in any debugger.  There is no interface defined
here at this time.  

.P
One problem with defining an interface is that operations are
machine dependent: they depend on the interpretation of
register numbers and the methods of getting values from the
environment the expression is applied to.

.P
It would be desirable to specify an interface.

.H 3 "Location List Internal-level Interface"

.H 4 "dwarf_get_loclist_entry()"
.DS
\f(CWint dwarf_get_loclist_entry(
        Dwarf_Debug dbg,
        Dwarf_Unsigned offset,
        Dwarf_Addr *hipc_offset,
        Dwarf_Addr *lopc_offset,
        Dwarf_Ptr *data,
        Dwarf_Unsigned *entry_len,
        Dwarf_Unsigned *next_entry,
        Dwarf_Error *error)\fP
.DE
\f(CWdwarf_dwarf_get_loclist_entry()\fP returns 
\f(CWDW_DLV_OK\fP if successful.
\f(CWDW_DLV_ERROR\fP is returned on error. 
The function reads 
a location list entry starting at \f(CWoffset\fP and returns 
through pointers (when successful)
the high pc \f(CWhipc_offset\fP, low pc 
\f(CWlopc_offset\fP, a pointer to the location description data 
\f(CWdata\fP, the length of the location description data 
\f(CWentry_len\fP, and the offset of the next location description 
entry \f(CWnext_entry\fP.  
When \f(CWhipc\fP and \f(CWlopc\fP are 
zero, this is the end of a particular location list.  

.P
The \f(CWhipc_offset\fP,
low pc \f(CWlopc_offset\fP are offsets from the beginning of the
current procedure, not genuine pc values.

.H 2 "Abbreviations access"
These are Internal-level Interface functions.  
Debuggers can ignore this.

.H 3 "dwarf_get_abbrev()"
.DS
\f(CWint dwarf_get_abbrev(
        Dwarf_Debug dbg,
        Dwarf_Unsigned offset,
        Dwarf_Abbrev   *returned_abbrev,
        Dwarf_Unsigned *length,
        Dwarf_Unsigned *attr_count,
        Dwarf_Error *error)\fP
.DE
The function \f(CWdwarf_get_abbrev()\fP returns
\f(CWDW_DLV_OK\fP and sets \f(CW*returned_fde\fP to
\f(CWDwarf_Abbrev\fP 
descriptor for an abbreviation at offset \f(CW*offset\fP in the abbreviations 
section (i.e .debug_abbrev) on success.  
The user is responsible for making sure that 
a valid abbreviation begins at \f(CWoffset\fP in the abbreviations section.  
The location pointed to by \f(CWlength\fP 
is set to the length in bytes of the abbreviation in the abbreviations 
section.  
The location pointed to by \f(CWattr_count\fP is set to the 
number of attributes in the abbreviation.  
An abbreviation entry with a 
length of 1 is the 0 byte of the last abbreviation entry of a compilation 
unit.
\f(CWdwarf_get_abbrev()\fP returns \f(CWDW_DLV_ERROR\fP on error.  

.H 3 "dwarf_get_abbrev_tag()"
.DS
\f(CWint dwarf_get_abbrev_tag(
        Dwarf_abbrev abbrev,
	Dwarf_Half  *return_tag,
        Dwarf_Error *error);\P
.DE
If successful,
\f(CWdwarf_get_abbrev_tag()\fP returns
\f(CWDW_DLV_OK\fP and sets \f(CW*return_tag\fP to
the \fItag\fP of 
the given abbreviation.
It returns \f(CWDW_DLV_ERROR\fP on error.
It never returns \f(CWDW_DLV_NO_ENTRY\fP.

.H 3 "dwarf_get_abbrev_code()"
.DS
\f(CWint dwarf_get_abbrev_code(
        Dwarf_abbrev     abbrev,
	Dwarf_Unsigned  *return_code,
        Dwarf_Error     *error);\P
.DE
If successful,
\f(CWdwarf_get_abbrev_code()\fP returns
\f(CWDW_DLV_OK\fP and sets \f(CW*return_code\fP to
the abbreviation code of
the given abbreviation.
It returns \f(CWDW_DLV_ERROR\fP on error.
It never returns \f(CWDW_DLV_NO_ENTRY\fP.

.H 3 "dwarf_get_abbrev_children_flag()"
.DS
\f(CWint dwarf_get_abbrev_children_flag(
        Dwarf_Abbrev abbrev,
	Dwarf_Signed  *returned_flag,
        Dwarf_Error *error)\fP
.DE
The function \f(CWdwarf_get_abbrev_children_flag()\fP returns 
\f(CWDW_DLV_OK\fP and sets \f(CWreturned_flag\fP to
\f(CWDW_children_no\fP (if the given abbreviation indicates that 
a die with that abbreviation has no children) or 
\f(CWDW_children_yes\fP (if the given abbreviation indicates that 
a die with that abbreviation has a child).  
It returns \f(CWDW_DLV_ERROR\fP on error.

.H 3 "dwarf_get_abbrev_entry()"
.DS
\f(CWint dwarf_get_abbrev_entry(
        Dwarf_Abbrev abbrev,
        Dwarf_Signed index,
        Dwarf_Half   *attr_num,
        Dwarf_Signed *form,
        Dwarf_Off *offset,
        Dwarf_Error *error)\fP

.DE
If successful,
\f(CWdwarf_get_abbrev_entry()\fP returns
\f(CWDW_DLV_OK\fP and sets \f(CW*attr_num\fP to the attribute code of
the attribute 
whose index is specified by \f(CWindex\fP in the given abbreviation.  
The index starts at 0.  
The location pointed to by \f(CWform\fP is set 
to the form of the attribute.  
The location pointed to by \f(CWoffset\fP 
is set to the byte offset of the attribute in the abbreviations section.  
It returns \f(CWDW_DLV_NO_ENTRY\fP if the index specified is outside
the range of attributes in this abbreviation.
It returns \f(CWDW_DLV_ERROR\fP on error.

.H 2 "String Section Operations"
The .debug_str section contains only strings.  Debuggers need 
never use this interface: it is only for debugging problems with 
the string section itself.  

.H 3 "dwarf_get_str()"
.DS
\f(CWint dwarf_get_str(
        Dwarf_Debug   dbg,
        Dwarf_Off     offset,
        char        **string,
	Dwarf_Signed  returned_str_len,
        Dwarf_Error *error)\fP
.DE
The function \f(CWdwarf_get_str()\fP returns
\f(CWDW_DLV_OK\fP and sets \f(CW*returned_str_len\fP to
the length of 
the string, not counting the null terminator, that begins at the offset 
specified by \f(CWoffset\fP in the .debug_str section.  
The location 
pointed to by \f(CWstring\fP is set to a pointer to this string.
The next string in the .debug_str
section begins at the previous \f(CWoffset\fP + 1 + \f(CW*returned_str_len\fP.
A zero-length string is NOT the end of the section.
If there is no .debug_str section, \f(CWDW_DLV_NO_ENTRY\fP is returned.
If there is an error, \f(CWDW_DLV_ERROR\fP is returned.

.H 2 "Address Range Operations"
These functions provide information about address ranges.  Address
ranges map ranges of pc values to the corresponding compilation-unit
die that covers the address range.

.H 3 "dwarf_get_aranges()"
.DS
\f(CWint dwarf_get_aranges(
        Dwarf_Debug dbg,
        Dwarf_Arange **aranges,
        Dwarf_Signed * returned_arange_count,
        Dwarf_Error *error)\fP
.DE
The function \f(CWdwarf_get_aranges()\fP returns 
\f(CWDW_DLV_OK\fP and sets \f(CW*returned_arange_count\fP to
the count of the
number of address ranges in the .debug_aranges section.  
It sets
\f(CW*aranges\fP to point to a block of \f(CWDwarf_Arange\fP 
descriptors, one for each address range.  
It returns \f(CWDW_DLV_ERROR\fP on error.
It returns \f(CWDW_DLV_NO_ENTRY\fP if there is no .debug_aranges
section.

.in +2
.DS
\f(CWDwarf_Signed cnt;
Dwarf_Arange *arang;

if ((dwarf_get_aranges(dbg, &arang,&cnt, &error)) == DW_DLV_OK) {

        for (i = 0; i < cnt; ++i) {
                /* use arang[i] */
                dwarf_dealloc(dbg, arang[i], DW_DLA_ARANGE);
        }
        dwarf_dealloc(dbg, arang, DW_DLA_LIST);
}\fP
.DE
.in -2


.DS
\f(CWint dwarf_get_arange(
        Dwarf_Arange *aranges,
        Dwarf_Unsigned arange_count,
        Dwarf_Addr address,
	Dwarf_Arange   *returned_arange,
        Dwarf_Error *error)\fP
.DE
The function \f(CWdwarf_get_arange()\fP takes as input a pointer 
to a block of \f(CWDwarf_Arange\fP pointers, and a count of the
number of descriptors in the block.  
It then searches for the
descriptor that covers the given \f(CWaddress\fP.  
If it finds
one, it returns
\f(CWDW_DLV_OK\fP and sets \f(CW*returned_arange\fP to
the descriptor. 
It returns \f(CWDW_DLV_ERROR\fP on error.
It returns \f(CWDW_DLV_NO_ENTRY\fP if there is no .debug_aranges
entry covering that address.


.H 3 "dwarf_get_cu_die_offset()"
.DS
\f(CWDwarf_Off dwarf_get_cu_die_offset(
        Dwarf_Arange arange,
        Dwarf_Off   *returned_offset,
        Dwarf_Error *error)\fP
.DE
The function \f(CWdwarf_get_cu_die_offset()\fP takes a
\f(CWDwarf_Arange\fP descriptor as input, and 
if successful returns
\f(CWDW_DLV_OK\fP and sets \f(CW*returned_offset\fP to
the offset
in the .debug_info section of the compilation-unit DIE for the 
compilation-unit represented by the given address range.
It returns \f(CWDW_DLV_ERROR\fP on error.

.H 3 "dwarf_get_arange_info()"
.DS
\f(CWint dwarf_get_arange_info(
        Dwarf_Arange arange,
        Dwarf_Addr *start,
        Dwarf_Unsigned *length,
        Dwarf_Off *cu_die_offset,
        Dwarf_Error *error)\fP
.DE
The function \f(CWdwarf_get_arange_info()\fP returns
\f(CWDW_DLV_OK\fP
and
stores the starting value of the address range in the location pointed 
to by \f(CWstart\fP, the length of the address range in the location 
pointed to by \f(CWlength\fP, and the offset in the .debug_info section 
of the compilation-unit DIE for the compilation-unit represented by the 
address range.
It returns \f(CWDW_DLV_ERROR\fP on error.

.H 2 "General Low Level Operations"
This function is low-level and intended for use only
by programs such as dwarf-dumpers.

.H 3 "dwarf_get_address_size()"
.DS
\f(CWint dwarf_get_address_size(Dwarf_Debug dbg,
	Dwarf_Half  *addr_size,
        Dwarf_Error *error)\fP
.DE
The function \f(CWdwarf_get_address_size()\fP 
returns \f(CWDW_DLV_OK\fP on success and sets 
the \f(CW*addr_size\fP
to the size in bytes of an address.
In case of error, it returns \f(CWDW_DLV_ERROR\fP
and does not set \f(CW*addr_size\fP.


.H 2 "Utility Operations"
These functions aid in the management of errors encountered when using 
functions in the \fIlibdwarf\fP library and releasing memory allocated 
as a result of a \fIlibdwarf\fP operation. 

.H 3 "dwarf_errno()"
.DS
\f(CWDwarf_Unsigned dwarf_errno(
        Dwarf_Error error)\fP
.DE
The function \f(CWdwarf_errno()\fP returns the error number corresponding 
to the error specified by \f(CWerror\fP.

.H 3 "dwarf_errmsg()"
.DS
\f(CWconst char* dwarf_errmsg(
        Dwarf_Error error)\fP
.DE
The function \f(CWdwarf_errmsg()\fP returns a pointer to a
null-terminated error message string corresponding to the error specified by 
\f(CWerror\fP.  
The string returned by \f(CWdwarf_errmsg()\fP 
should not be deallocated using \f(CWdwarf_dealloc()\fP.

.P
The set of errors 
enumerated in Figure \n(aX below were defined in Dwarf 1.
These errors are not used by the current implementation
of Dwarf 2.  
.DS
.TS
center box, tab(:);
lfB lfB 
l l.
SYMBOLIC NAME:DESCRIPTION
_
DW_DLE_NE:No error (0)
DW_DLE_VMM:Version of DWARF information newer than libdwarf
DW_DLE_MAP:Memory map failure
DW_DLE_LEE:Propagation of libelf error
DW_DLE_NDS:No debug section
DW_DLE_NLS:No line section
DW_DLE_ID:Requested information not associated with descriptor
DW_DLE_IOF:I/O failure
DW_DLE_MAF:Memory allocation failure
DW_DLE_IA:Invalid argument
DW_DLE_MDE:Mangled debugging entry
DW_DLE_MLE:Mangled line number entry
DW_DLE_FNO:File descriptor does not refer to an open file
DW_DLE_FNR:File is not a regular file
DW_DLE_FWA:File is opened with wrong access
DW_DLE_NOB:File is not an object file
DW_DLE_MOF:Mangled object file header
DW_DLE_EOLL:End of location list entries
DW_DLE_NOLL:No location list section
DW_DLE_BADOFF:Invalid offset
DW_DLE_EOS:End of section
DW_DLE_ATRUNC:Abbreviations section appears truncated
DW_DLE_BADBITC:Address size passed to dwarf bad
.TE
.FG "List of Dwarf Error Codes"
.DE

The set of errors returned by SGI \f(CWLibdwarf\fP functions
is listed below.
Some of the errors are SGI specific.

.DS
.TS
center box, tab(:);
lfB lfB
l l.
SYMBOLIC NAME:DESCRIPTION
_
DW_DLE_DBG_ALLOC:Could not allocate Dwarf_Debug struct
DW_DLE_FSTAT_ERROR:Error in fstat()-ing object
DW_DLE_FSTAT_MODE_ERROR:Error in mode of object file
DW_DLE_INIT_ACCESS_WRONG:Incorrect access to dwarf_init()
DW_DLE_ELF_BEGIN_ERROR:Error in elf_begin() on object
DW_DLE_ELF_GETEHDR_ERROR:Error in elf_getehdr() on object
DW_DLE_ELF_GETSHDR_ERROR:Error in elf_getshdr() on object
DW_DLE_ELF_STRPTR_ERROR:Error in elf_strptr() on object
DW_DLE_DEBUG_INFO_DUPLICATE:Multiple .debug_info sections
DW_DLE_DEBUG_INFO_NULL:No data in .debug_info section
DW_DLE_DEBUG_ABBREV_DUPLICATE:Multiple .debug_abbrev sections
DW_DLE_DEBUG_ABBREV_NULL:No data in .debug_abbrev section
DW_DLE_DEBUG_ARANGES_DUPLICATE:Multiple .debug_arange sections
DW_DLE_DEBUG_ARANGES_NULL:No data in .debug_arange section
DW_DLE_DEBUG_LINE_DUPLICATE:Multiple .debug_line sections
DW_DLE_DEBUG_LINE_NULL:No data in .debug_line section
DW_DLE_DEBUG_LOC_DUPLICATE:Multiple .debug_loc sections
DW_DLE_DEBUG_LOC_NULL:No data in .debug_loc section
DW_DLE_DEBUG_MACINFO_DUPLICATE:Multiple .debug_macinfo sections
DW_DLE_DEBUG_MACINFO_NULL:No data in .debug_macinfo section
DW_DLE_DEBUG_PUBNAMES_DUPLICATE:Multiple .debug_pubnames sections
DW_DLE_DEBUG_PUBNAMES_NULL:No data in .debug_pubnames section
DW_DLE_DEBUG_STR_DUPLICATE:Multiple .debug_str sections
DW_DLE_DEBUG_STR_NULL:No data in .debug_str section
DW_DLE_CU_LENGTH_ERROR:Length of compilation-unit bad
DW_DLE_VERSION_STAMP_ERROR:Incorrect Version Stamp
DW_DLE_ABBREV_OFFSET_ERROR:Offset in .debug_abbrev bad
DW_DLE_ADDRESS_SIZE_ERROR:Size of addresses in target bad
DW_DLE_DEBUG_INFO_PTR_NULL:Pointer into .debug_info in DIE null
DW_DLE_DIE_NULL:Null Dwarf_Die
DW_DLE_STRING_OFFSET_BAD:Offset in .debug_str bad
DW_DLE_DEBUG_LINE_LENGTH_BAD:Length of .debug_line segment bad
DW_DLE_LINE_PROLOG_LENGTH_BAD:Length of .debug_line prolog bad
DW_DLE_LINE_NUM_OPERANDS_BAD:Number of operands to line instr bad
DW_DLE_LINE_SET_ADDR_ERROR:Error in DW_LNE_set_address instruction
DW_DLE_LINE_EXT_OPCODE_BAD:Error in DW_EXTENDED_OPCODE instruction
DW_DLE_DWARF_LINE_NULL:Null Dwarf_line argument
DW_DLE_INCL_DIR_NUM_BAD:Error in included directory for given line
DW_DLE_LINE_FILE_NUM_BAD:File number in .debug_line bad
DW_DLE_ALLOC_FAIL:Failed to allocate required structs
DW_DLE_DBG_NULL:Null Dwarf_Debug argument
DW_DLE_DEBUG_FRAME_LENGTH_BAD:Error in length of frame
DW_DLE_FRAME_VERSION_BAD:Bad version stamp for frame
DW_DLE_CIE_RET_ADDR_REG_ERROR:Bad register specified for return address
DW_DLE_FDE_NULL:Null Dwarf_Fde argument
DW_DLE_FDE_DBG_NULL:No Dwarf_Debug associated with FDE
DW_DLE_CIE_NULL:Null Dwarf_Cie argument
DW_DLE_CIE_DBG_NULL:No Dwarf_Debug associated with CIE
DW_DLE_FRAME_TABLE_COL_BAD:Bad column in frame table specified
.TE
.FG "List of Dwarf 2 Error Codes (continued)"
.DE

.DS
.TS
center box, tab(:);
lfB lfB
l l.
SYMBOLIC NAME:DESCRIPTION
_
DW_DLE_PC_NOT_IN_FDE_RANGE:PC requested not in address range of FDE
DW_DLE_CIE_INSTR_EXEC_ERROR:Error in executing instructions in CIE
DW_DLE_FRAME_INSTR_EXEC_ERROR:Error in executing instructions in FDE
DW_DLE_FDE_PTR_NULL:Null Pointer to Dwarf_Fde specified
DW_DLE_RET_OP_LIST_NULL:No location to store pointer to Dwarf_Frame_Op
DW_DLE_LINE_CONTEXT_NULL:Dwarf_Line has no context
DW_DLE_DBG_NO_CU_CONTEXT:dbg has no CU context for dwarf_siblingof()
DW_DLE_DIE_NO_CU_CONTEXT:Dwarf_Die has no CU context
DW_DLE_FIRST_DIE_NOT_CU:First DIE in CU not DW_TAG_compilation_unit
DW_DLE_NEXT_DIE_PTR_NULL:Error in moving to next DIE in .debug_info
DW_DLE_DEBUG_FRAME_DUPLICATE:Multiple .debug_frame sections
DW_DLE_DEBUG_FRAME_NULL:No data in .debug_frame section
DW_DLE_ABBREV_DECODE_ERROR:Error in decoding abbreviation
DW_DLE_DWARF_ABBREV_NULL:Null Dwarf_Abbrev specified
DW_DLE_ATTR_NULL:Null Dwarf_Attribute specified
DW_DLE_DIE_BAD:DIE bad
DW_DLE_DIE_ABBREV_BAD:No abbreviation found for code in DIE
DW_DLE_ATTR_FORM_BAD:Inappropriate attribute form for attribute
DW_DLE_ATTR_NO_CU_CONTEXT:No CU context for Dwarf_Attribute struct
DW_DLE_ATTR_FORM_SIZE_BAD:Size of block in attribute value bad
DW_DLE_ATTR_DBG_NULL:No Dwarf_Debug for Dwarf_Attribute struct
DW_DLE_BAD_REF_FORM:Inappropriate form for reference attribute
DW_DLE_ATTR_FORM_OFFSET_BAD:Offset reference attribute outside current CU
DW_DLE_LINE_OFFSET_BAD:Offset of lines for current CU outside .debug_line
DW_DLE_DEBUG_STR_OFFSET_BAD:Offset into .debug_str past its end
DW_DLE_STRING_PTR_NULL:Pointer to pointer into .debug_str NULL
DW_DLE_PUBNAMES_VERSION_ERROR:Version stamp of pubnames incorrect
DW_DLE_PUBNAMES_LENGTH_BAD:Read pubnames past end of .debug_pubnames
DW_DLE_GLOBAL_NULL:Null Dwarf_Global specified
DW_DLE_GLOBAL_CONTEXT_NULL:No context for Dwarf_Global given
DW_DLE_DIR_INDEX_BAD:Error in directory index read
DW_DLE_LOC_EXPR_BAD:Bad operator read for location expression
DW_DLE_DIE_LOC_EXPR_BAD:Expected block value for attribute not found
DW_DLE_OFFSET_BAD:Offset for next compilation-unit in .debug_info bad
DW_DLE_MAKE_CU_CONTEXT_FAIL:Could not make CU context
DW_DLE_ARANGE_OFFSET_BAD:Offset into .debug_info in .debug_aranges bad
DW_DLE_SEGMENT_SIZE_BAD:Segment size should be 0 for MIPS processors
DW_DLE_ARANGE_LENGTH_BAD:Length of arange section in .debug_arange bad
DW_DLE_ARANGE_DECODE_ERROR:Aranges do not end at end of .debug_aranges
DW_DLE_ARANGES_NULL:NULL pointer to Dwarf_Arange specified
DW_DLE_ARANGE_NULL:NULL Dwarf_Arange specified
DW_DLE_NO_FILE_NAME:No file name for Dwarf_Line struct
DW_DLE_NO_COMP_DIR:No Compilation directory for compilation-unit
DW_DLE_CU_ADDRESS_SIZE_BAD:CU header address size not match Elf class
DW_DLE_ELF_GETIDENT_ERROR:Error in elf_getident() on object
DW_DLE_NO_AT_MIPS_FDE:DIE does not have DW_AT_MIPS_fde attribute
DW_DLE_NO_CIE_FOR_FDE:No CIE specified for FDE
DW_DLE_DIE_ABBREV_LIST_NULL:No abbreviation for the code in DIE found
DW_DLE_DEBUG_FUNCNAMES_DUPLICATE:Multiple .debug_funcnames sections
DW_DLE_DEBUG_FUNCNAMES_NULL:No data in .debug_funcnames section
.TE
.FG "List of Dwarf 2 Error Codes (continued)"
.DE

.DS
.TS
center box, tab(:);
lfB lfB
l l.
SYMBOLIC NAME:DESCRIPTION
_
DW_DLE_DEBUG_FUNCNAMES_VERSION_ERROR:Version stamp in .debug_funcnames bad
DW_DLE_DEBUG_FUNCNAMES_LENGTH_BAD:Length error in reading .debug_funcnames
DW_DLE_FUNC_NULL:NULL Dwarf_Func specified
DW_DLE_FUNC_CONTEXT_NULL:No context for Dwarf_Func struct
DW_DLE_DEBUG_TYPENAMES_DUPLICATE:Multiple .debug_typenames sections
DW_DLE_DEBUG_TYPENAMES_NULL:No data in .debug_typenames section
DW_DLE_DEBUG_TYPENAMES_VERSION_ERROR:Version stamp in .debug_typenames bad
DW_DLE_DEBUG_TYPENAMES_LENGTH_BAD:Length error in reading .debug_typenames
DW_DLE_TYPE_NULL:NULL Dwarf_Type specified
DW_DLE_TYPE_CONTEXT_NULL:No context for Dwarf_Type given
DW_DLE_DEBUG_VARNAMES_DUPLICATE:Multiple .debug_varnames sections
DW_DLE_DEBUG_VARNAMES_NULL:No data in .debug_varnames section
DW_DLE_DEBUG_VARNAMES_VERSION_ERROR:Version stamp in .debug_varnames bad
DW_DLE_DEBUG_VARNAMES_LENGTH_BAD:Length error in reading .debug_varnames
DW_DLE_VAR_NULL:NULL Dwarf_Var specified
DW_DLE_VAR_CONTEXT_NULL:No context for Dwarf_Var given
DW_DLE_DEBUG_WEAKNAMES_DUPLICATE:Multiple .debug_weaknames section
DW_DLE_DEBUG_WEAKNAMES_NULL:No data in .debug_varnames section
DW_DLE_DEBUG_WEAKNAMES_VERSION_ERROR:Version stamp in .debug_varnames bad
DW_DLE_DEBUG_WEAKNAMES_LENGTH_BAD:Length error in reading .debug_weaknames
DW_DLE_WEAK_NULL:NULL Dwarf_Weak specified
DW_DLE_WEAK_CONTEXT_NULL:No context for Dwarf_Weak given
.TE
.FG "List of Dwarf 2 Error Codes"
.DE

This list of errors is not necessarily complete;
additional errors
might be added when functionality to create debugging information 
entries are added to \fIlibdwarf\fP and by the implementors of 
\fIlibdwarf\fP to describe internal errors not addressed by the 
above list.
Some of the above errors may be unused.
Errors may not have the same meaning in different implementations.

.H 3 "dwarf_seterrhand()"
.DS
\f(CWDwarf_Handler dwarf_seterrhand(
        Dwarf_Debug dbg,
        Dwarf_Handler errhand)\fP
.DE
The function \f(CWdwarf_seterrhand()\fP replaces the error handler 
(see \f(CWdwarf_init()\fP) with \f(CWerrhand\fP.  The old error handler 
is returned.  This function is currently unimplemented.

.H 3 "dwarf_seterrarg()"
.DS
\f(CWDwarf_Ptr dwarf_seterrarg(
        Dwarf_Debug dbg,
        Dwarf_Ptr errarg)\fP
.DE
The function \f(CWdwarf_seterrarg()\fP replaces the pointer to the 
error handler communication area (see \f(CWdwarf_init()\fP) with 
\f(CWerrarg\fP.  A pointer to the old area is returned.  This
function is currently unimplemented.

.H 3 "dwarf_dealloc()"
.DS
\f(CWvoid dwarf_dealloc(
        Dwarf_Debug dbg,
        void* space, 
        Dwarf_Unsigned type)\fP
.DE
The function \f(CWdwarf_dealloc\fP frees the dynamic storage pointed
to by \f(CWspace\fP, and allocated to the given \f(CWDwarf_Debug\fP.
The argument \f(CWtype\fP is an integer code that specifies the allocation 
type of the region pointed to by the \f(CWspace\fP.  Refer to section 
4 for details on \fIlibdwarf\fP memory management.

.SK
.S
.TC 1 1 4
.CS
