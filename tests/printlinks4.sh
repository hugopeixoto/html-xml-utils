:
trap 'rm $TMP1 $TMP2' 0
TMP1=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP2=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1

./hxprintlinks -b "http://example/base/" >$TMP1 <<EOF
<head profile="http://www.microformats.org/wiki/hcard-profile">
<link rel=alternate href="a.atom">
<p><img src="../b.png" alt="">
</html>
EOF

cat >$TMP2 <<EOF
<head profile="http://www.microformats.org/wiki/hcard-profile">
<link rel="alternate" href="a.atom">
<p><img src="../b.png" alt="">
<ol class="urllist">
<li><a class="profile" href="http://www.microformats.org/wiki/hcard-profile">http://www.microformats.org/wiki/hcard-profile</a></li>
<li><a class="href" href="http://example/base/a.atom">http://example/base/a.atom</a></li>
<li><a class="src" href="http://example/b.png">http://example/b.png</a></li>
</ol>
</html>
EOF

cmp -s $TMP1 $TMP2
