:
trap 'rm $TMP1 $TMP2 $TMP3 $TMP4' 0
TMP1=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP2=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP3=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP4=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1

echo 'Test' >$TMP4

echo '<!-- include sub2 -->' >$TMP3

echo '<!-- include sub -->' | ./hxincl -s sub=$TMP3 -s sub2=$TMP4 -b /tmp/here >$TMP1

(echo '<!--begin-include sub --><!--begin-include sub2 -->Test'
 echo '<!--end-include-->'
 echo '<!--end-include-->'
) >$TMP2

cmp -s $TMP1 $TMP2
