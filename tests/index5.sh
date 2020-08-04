:
trap 'rm $TMP1 $TMP2 $TMP3 $TMP4 $TMP5' 0
TMP1=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP2=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP3=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP4=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP5=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1

# Make a file to index
echo '<title>This document</title><dfn>term1</dfn> <dfn>term2</dfn>' >$TMP1

# Initialize a database with some terms
cat >$TMP2 <<EOF
term3	2	bar#term3	#	Section 1	Other document
term4	2	bar#term4	#	Section 2	Other document
EOF

# Create the expected result database
cat >$TMP3 <<EOF
term2	2	foo#term2	#	This document	This document
term1	2	foo#term1	#	This document	This document
EOF
cat $TMP2 >>$TMP3

# Call hxindex
./hxindex -i $TMP2 -b foo $TMP1 >/dev/null

# Check. The order of the terms in the database may differ.
sort $TMP2 >$TMP4
sort $TMP3 >$TMP5
diff -u $TMP4 $TMP5
