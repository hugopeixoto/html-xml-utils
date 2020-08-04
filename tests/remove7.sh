:

trap 'rm $TMP1 $TMP2' 0
TMP1=`mktemp /tmp/tmpXXXXXX` || exit 1
TMP2=`mktemp /tmp/tmpXXXXXX` || exit 1

cat >$TMP1 <<EOF
<doc>
  <nested>
    <abc id="a1"/>
    <def>def</def>
  </nested>
</doc>
EOF

./hxremove :root :root <$TMP1 >$TMP2
diff -u $TMP2 $TMP2
