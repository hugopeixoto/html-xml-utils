:
trap 'rm -r $TMP1 $TMP2 $TMP3' 0
TMP1=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP2=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP3=`mktemp -d /tmp/tmp.XXXXXXXXXX` || exit 1

p=$PWD
cd $TMP3
mkdir dir

echo 'Test1' >file1
echo '<!-- include ../file3 -->' >dir/file2
echo 'Test3' >file3
(echo '<!-- include file1 -->'
 echo '<!-- include dir/file2 -->'
) >file0

$p/hxincl -M target file0 >$TMP1

(echo "target: \\"
 echo " file1 \\"
 echo " dir/file2 \\"
 echo " file3"
) >$TMP2

cmp -s $TMP1 $TMP2
