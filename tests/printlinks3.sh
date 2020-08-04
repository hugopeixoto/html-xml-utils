:
trap 'rm $TMP1 $TMP2' 0
TMP1=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP2=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1

./hxprintlinks >$TMP1 <<EOF
<head profile="http://www.microformats.org/wiki/hcard-profile">
<link rel=alternate href="a.atom">
<p><a href="a.atom">atom</a>
</html>
EOF

cat >$TMP2 <<EOF
<head profile="http://www.microformats.org/wiki/hcard-profile">
<link rel="alternate" href="a.atom">
<p><a href="a.atom">atom</a>
<ol class="urllist">
<li><a class="profile" href="http://www.microformats.org/wiki/hcard-profile">http://www.microformats.org/wiki/hcard-profile</a></li>
<li><a class="href" href="a.atom">a.atom</a></li>
<li><a class="href" href="a.atom">a.atom</a></li>
</ol>
</html>
EOF

cmp -s $TMP1 $TMP2
