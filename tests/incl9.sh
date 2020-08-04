:
trap 'rm -r $TMP1 $TMP2 $TMP3' 0
TMP1=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP2=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP3=`mktemp -d /tmp/tmp.XXXXXXXXXX` || exit 1

echo 'Test1' >$TMP3/file1
echo 'Test2' >$TMP3/file2
echo 'Test3' >$TMP3/file3
echo '<!-- include file1 -->' >$TMP3/file0
echo '<!-- include file2 -->' >>$TMP3/file0
echo '<!-- include file3 -->' >>$TMP3/file0

./hxincl -M foo -s sub=$TMP3 $TMP3/file0 >$TMP1

echo "foo: \\" >$TMP2
echo " $TMP3/file1 \\" >>$TMP2
echo " $TMP3/file2 \\" >>$TMP2
echo " $TMP3/file3" >>$TMP2

cmp -s $TMP1 $TMP2
