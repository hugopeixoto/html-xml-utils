:
trap 'rm $TMP1 $TMP2 $TMP3' 0
TMP1=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP2=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP3=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1

echo '<!- this is not a comment -->' >$TMP3

echo '<html><body><p>&lt;!- this is not a comment -->' >$TMP2
echo '</p></body></html>' >>$TMP2

./hxclean $TMP3 >$TMP1
echo >>$TMP1		       # Add a newline at the end

cmp -s $TMP1 $TMP2
