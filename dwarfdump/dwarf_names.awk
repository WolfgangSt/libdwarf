# Portions Copyright 2007 Sun Microsystems, Inc. All rights reserved.
#
# Print routines to return constant name for associated value.
# The input is dwarf.h
# For each set of names with a common prefix, we create a routine
# to return the name given the value.
# Also print header file that gives prototypes of routines.
# To handle cases where there are multiple names for a single
# value (DW_AT_* has some due to ambiguities in the DWARF2 spec)
# we take the first of a given value as the definitive name.
# TAGs, Attributes, etc are given distinct checks.

# 
# to test the code generated in dwarf_names_new.c and dwarf_names_new.h
# you can use this test program
#
#   #include "dwarf.h"
#   #define NULL (char*)0
#   #define DWARF_PRINT_PREFIX mydw_
#   #include "dwarf_names_new.h"
#   int main()
#   {
#      printf(mydw_get_TAG_name(DW_TAG_label));
#   }
#   #include "dwarf_names_new.c"
# 



BEGIN {
	prefix = "foo"
	prefix_id = "foo"
	prefix_len = length(prefix)
	dw_prefix = "DW_"
	dw_len = length(dw_prefix)
	start_routine = 0
	printf "#include \"globals.h\"\n\n"
	printf "#include \"makename.h\"\n\n"
	header = "dwarf_names.h"
        enum_header = "dwarf_names_enum.h"
        dwarf_names_new  = "dwarf_names_new.c"
	dwarf_names_newh = "dwarf_names_new.h"
	printf "/* automatically generated routines */\n" > header
	dup_arr["0"] = ""
        printf "/* automatically generated enums for dwarf defines */\n" > enum_header

	printf "/* automatically generated */\n" > dwarf_names_new
	printf "/* make sure to include the corresponding */\n" > dwarf_names_new
	printf "/* header here, so DWPREFIX is defined */\n" > dwarf_names_new

	printf "/* automatically generated */\n" > dwarf_names_newh
	printf "/* define DWARF_PRINT_PREFIX before this point */\n" > dwarf_names_newh
	printf "/* if you want one */\n" > dwarf_names_newh
	printf "#define dw_glue(x,y) x##y\n" > dwarf_names_newh
	printf "#define dw_glue2(x,y) dw_glue(x,y)\n" > dwarf_names_newh
	printf "#define DWPREFIX(x) dw_glue2(DWARF_PRINT_PREFIX,x)\n" > dwarf_names_newh

        printf "#ifndef __DWARF_NAMES_ENUM_H__\n" > enum_header;
        printf "#define __DWARF_NAMES_ENUM_H__\n" > enum_header;
}
{
	if (skipit && $1 == "#endif") {
		skipit = 0
		next
	}
	if ($2 == 0 || skipit) {
		# if 0, skip to endif
		skipit = 1
		next
	}
	if ($1 == "#define") {
		if (substr($2,1,prefix_len) != prefix) {
			# new prefix
			if (substr($2,1,dw_len) != dw_prefix) {
				# skip
				next
			} else if (substr($2,1,dw_len+3) == "DW_CFA") {
				# skip, cause numbers conflict
				# (have both high-order and low-order bits)
				next
			} else {
				# New prefix, empty the dup_arr
				for (k in dup_arr)
					dup_arr[k] = ""
				if (start_routine) {
printf "\tdefault:; }\n\treturn NULL;\n" > dwarf_names_new;
printf "}\n\n" > dwarf_names_new;
					# end routine
					printf "\tdefault:\n"
printf "\t\t{ \n"
printf "\t\t    char buf[100]; \n"
printf "\t\t    char *n; \n"
printf "\t\t    snprintf(buf,sizeof(buf),\"<Unknown %s value 0x%%x>\",(int)val);\n",prefix_id
printf "\t\t fprintf(stderr,\"%s of %%d (0x%%x) is unknown to dwarfdump. \" \n ", prefix_id
printf "\t\t \"Continuing. \\n\",(int)val,(int)val );  \n"
printf "\t\t    n = makename(buf);\n"
printf "\t\t    return n; \n"
printf "\t\t} \n"
					printf "\t}\n"
					printf "/*NOTREACHED*/\n"
					printf "}\n\n"


				        printf "%s\n", last_entry >> enum_header
				        printf "};\n" >> enum_header
				        last_entry = "";
				}
				start_routine = 1
				post_dw = substr($2,dw_len+1, length($2))
				second_underscore = index(post_dw,"_")
				prefix = substr($2,1,second_underscore+dw_len)
				prefix_len = length(prefix)
				# prefix id is unique part after DW_, e.g. LANG
				prefix_id = substr(prefix,dw_len+1,prefix_len-dw_len-1)
				printf "/* ARGSUSED */\n"
				printf "extern string\n"

				printf "get_%s_name (Dwarf_Debug dbg, Dwarf_Half val)\n", prefix_id
				printf "{\n"
				printf "\tswitch (val) {\n"
				printf "extern string get_%s_name (Dwarf_Debug dbg, Dwarf_Half val);\n\n", prefix_id >> header

				printf "const char * DWPREFIX(get_%s_name) (unsigned int val){ \n", prefix_id >> dwarf_names_new
				printf "\tswitch (val) {\n" >> dwarf_names_new
				
				printf "extern string get_%s_name (Dwarf_Debug dbg, Dwarf_Half val);\n\n", prefix_id >> header

				printf "const char * DWPREFIX(get_%s_name) (unsigned int);\n", prefix_id >> dwarf_names_newh


printf "\nenum Dwarf_%s_e {\n", prefix_id >> enum_header
			}
		}
		if (substr($2,1,prefix_len) == prefix) {
			if (substr($2,1,dw_len+8) == "DW_CHILDREN" \
			    || substr($2,1,dw_len+8) == "DW_children" \
			    || substr($2,1,dw_len+4) == "DW_ADDR") {
				main_part = substr($2,dw_len+1, length($2))
			}
			else {
				post_dw = substr($2,dw_len+1, length($2))
				second_underscore = index(post_dw,"_")
				main_part = substr($2,dw_len+second_underscore+1, length($2))
			}
			if( dup_arr[$3] != $3 ) {
			  # Take first of those with identical value,
			  # ignore others.
			  dup_arr[$3] = $3
			  printf "\tcase %s:\n", $2
			  printf "\t\tif (ellipsis)\n"
			  printf "\t\t\treturn \"%s\";\n", main_part
			  printf "\t\telse\n"
			  printf "\t\t\treturn \"%s\";\n", $2

			  printf "\tcase %s: return \"%s\";\n", $2, $2 >> dwarf_names_new


		        }

			# printf "\t%-40s = %s,\n", $2, $3 > enum_header
			if (last_entry != "") {
			     printf "%s,\n", last_entry > enum_header
			}
			last_entry = sprintf("\t%-40s = %s", $2, $3)
                        if ($2 ~ /FRAME_LAST_REG_NUM/) {
			   last_entry = sprintf("\t%-40s", $2)
                        }
		}
	}
}
END {
	if (start_routine) {
		printf "\tdefault:\n"
		printf "\t\tprint_error(dbg, \"get_%s_name unexpected value\",DW_DLV_OK, err);\n", prefix_id
		printf "\t}\n"
		printf "\t return \"unknown-name-dwarf-error\";\n"
		printf "}\n\n"

		printf "%s\n", last_entry >> enum_header
		printf "};\n" >> enum_header
		last_entry = "";

printf "\tdefault:; }\n\treturn NULL;\n}\n" > dwarf_names_new

	}
        printf "#endif /* __DWARF_NAMES_ENUM_H__ */\n" > enum_header
}

