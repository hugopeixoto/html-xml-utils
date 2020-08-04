:
trap 'rm $TMP1 $TMP2' 0
TMP1=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP2=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1

echo -e "abcdefghijklmnopqrstuvwxyz\0000\0002\0003\0175\0176\0177" >$TMP1

./xml2asc <$TMP1 >$TMP2

cmp -s $TMP1 $TMP2
