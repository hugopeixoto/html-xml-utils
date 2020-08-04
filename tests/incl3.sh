:
trap 'rm $TMP1 $TMP2 $TMP3' 0
TMP1=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP2=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP3=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1

echo "Test" >$TMP3

echo '<!-- include sub -->' | ./hxincl -s sub=$TMP3 >$TMP1

(echo '<!--begin-include sub -->Test'
 echo '<!--end-include-->'
) >$TMP2

cmp -s $TMP1 $TMP2
