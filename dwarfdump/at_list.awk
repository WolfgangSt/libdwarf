# print code to return attribute name from list of attrs in dwarf.h (the input)
BEGIN {
	printf "static int list_of_attrs[] = {\n"
}
{
	prefix = "DW_AT_"
	prefix_len = length(prefix)
	if ($1 == "#define" && substr($2,1,prefix_len) == prefix) {
		printf "\t%s,\n", $2
	}
}
END {
	printf "\t0\n"		# last value
	printf "};\n"
}

