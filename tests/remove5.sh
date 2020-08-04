:

trap 'rm $TMP1 $TMP2 $TMP3' 0
TMP1=`mktemp /tmp/tmpXXXXXX` || exit 1
TMP2=`mktemp /tmp/tmpXXXXXX` || exit 1
TMP3=`mktemp /tmp/tmpXXXXXX` || exit 1

cat >$TMP1 <<EOF
<ol>
<li id=l1>item 1</li>
<li>item 2</li>
<li id=l3>item 3</li>
<li>item 4</li>
<li>item 5</li>
</ol>
EOF

./hxremove 'ol #l1, ol #l3' <$TMP1 >$TMP2
./hxremove 'li[id]'         <$TMP1 >$TMP3
diff -u $TMP2 $TMP3
