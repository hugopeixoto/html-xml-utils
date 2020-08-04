:
trap 'rm -rf $TMP1 $TMP2 $TMP3' 0
TMP1=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP2=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP3=`mktemp -d /tmp/tmp.XXXXXXXXXX` || exit 1

mkdir -p $TMP3/dir1/dir2
echo 'Test' >$TMP3/dir1/test1
echo '<!-- include ../test1 -->' >$TMP3/dir1/dir2/test2
echo '<!-- include dir1/dir2/test2 -->' | ./hxincl -b $TMP3/. >$TMP1

(echo '<!--begin-include dir1/dir2/test2 --><!--begin-include ../test1 -->Test'
 echo '<!--end-include-->'
 echo '<!--end-include-->'
) >$TMP2

cmp -s $TMP1 $TMP2
