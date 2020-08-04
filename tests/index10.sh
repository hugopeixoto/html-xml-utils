:
trap 'rm $TMP1 $TMP2 $TMP3 $TMP4 $TMP5 $TMP6' 0
TMP1=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP2=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP3=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP4=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP5=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP6=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1

# Make three files to index
echo '<title>This document</title><dfn>term1</dfn> <dfn>term2</dfn>' >$TMP1
echo '<title>That document</title><dfn>term1</dfn> <dfn>term3</dfn>' >$TMP2
echo '<title>Index</title><body><!--index-->' >$TMP3

# Call hxindex on all three files to create a term database in $TMP4
./hxindex -i $TMP4 -b foo1 -t $TMP1 >/dev/null
./hxindex -i $TMP4 -b foo2 -t $TMP2 >/dev/null
./hxindex -i $TMP4 -b foo3 -t $TMP3 >/dev/null

# Call hxindex again to create the index
./hxindex -i $TMP4 -b foo3 -t $TMP3 >$TMP5

# Add a newline
echo >>$TMP5

cat $TMP4

# Create the expected result
cat >$TMP6 <<EOF
<html><head><title>Index</title></head><body><!--begin-index-->
<ul class="indexlist">
<li>term1, <a href="foo1#term1"><strong>#</strong></a>, <a href="foo2#term1"><strong>#</strong></a>
<li>term2, <a href="foo1#term2"><strong>#</strong></a>
<li>term3, <a href="foo2#term3"><strong>#</strong></a>
</ul><!--end-index--></body></html>
EOF

# Check.
diff -u $TMP6 $TMP5
