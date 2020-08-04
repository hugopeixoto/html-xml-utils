:

trap 'rm $TMP1 $TMP2 $TMP3' 0
TMP1=`mktemp /tmp/tmp.XXXXXXXX` || exit 1
TMP2=`mktemp /tmp/tmp.XXXXXXXX` || exit 1
TMP3=`mktemp /tmp/tmp.XXXXXXXX` || exit 1

cat >$TMP1 <<EOF
<doc>
<!-- do not remove this -->
<foo>
<!-- remove this -->
</foo>
<!-- do not remove this -->
</doc>
EOF

cat >$TMP2 <<EOF
<doc>
<!-- do not remove this -->

<!-- do not remove this -->
</doc>
EOF

./hxremove foo <$TMP1 >$TMP3

cmp -s $TMP2 $TMP3
