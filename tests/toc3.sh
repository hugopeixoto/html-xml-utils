:
trap 'rm $TMP1 $TMP2' 0
TMP1=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP2=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1

./hxtoc -d -t -f >$TMP1 <<-EOF
	<title>Test</title>
	<h1>Document <b>heading</b></h1>
	<!--toc-->
	<h2>Second-level <img src="" alt="heading"></h2>
	<h3>Third-level <bdo dir=ltr>heading</bdo></h3>
	<h3>Another <abbr title="none" dir=ltr>Third-level</abbr> heading</h3>
EOF
echo >>$TMP1			# Add newline

cat >$TMP2 <<-EOF
<html><head><title>Test</title></head><body><h1 id="document-heading">Document <b>heading</b></h1><!--begin-toc-->
<ul class="toc">
<li><a href="#document-heading">Document heading</a>
 <ul class="toc">
 <li><a href="#second-level-">Second-level heading</a>
  <ul class="toc">
  <li><a href="#third-level-heading">Third-level <bdo dir="ltr">heading</bdo></a>
  <li><a href="#another-third-level-heading">Another <span dir="ltr">Third-level</span> heading</a>
  </ul>
 </ul></ul>
<!--end-toc--><h2 id="second-level-">Second-level <img src="" alt="heading"></h2><h3 id="third-level-heading">Third-level <bdo dir="ltr">heading</bdo></h3><h3 id="another-third-level-heading">Another <abbr title="none" dir="ltr">Third-level</abbr> heading</h3></body></html>
EOF

cmp -s $TMP1 $TMP2
