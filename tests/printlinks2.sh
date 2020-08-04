:
trap 'rm $TMP1 $TMP2' 0
TMP1=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP2=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1

./hxprintlinks >$TMP1 <<EOF
<link rel=alternate href="a.atom">
<p><a href="a.atom">atom</a>
</body>
EOF

cat >$TMP2 <<EOF
<link rel="alternate" href="a.atom">
<p><a href="a.atom">atom</a>
<ol class="urllist">
<li><a class="href" href="a.atom">a.atom</a></li>
<li><a class="href" href="a.atom">a.atom</a></li>
</ol>
</body>
EOF

cmp -s $TMP1 $TMP2
