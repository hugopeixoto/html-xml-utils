:
trap 'rm $TMP1 $TMP2' 0
TMP1=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1

echo -e "\0200" >$TMP1

! ./xml2asc <$TMP1 >/dev/null
