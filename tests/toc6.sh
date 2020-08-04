:
trap 'rm $TMP1 $TMP2' 0
TMP1=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1
TMP2=`mktemp /tmp/tmp.XXXXXXXXXX` || exit 1

./hxtoc -d -t -h 8 >$TMP1 <<-EOF
	<title>Test</title>
	<h1>Document heading</h1>
	<!--toc-->
	<section id=sec2>
	<h1>Second-level heading</h1>
	<section id=sec3>
	<h1>Third-level heading</h1>
	<section id=sec4>
	<h1>Fourth-level heading</h1>
	<section id=sec5>
	<h1>Fifth-level heading</h1>
	<section id=sec6>
	<h1>Sixth-level heading</h1>
	<section id=sec7>
	<h1>Seventh-level heading</h1>
	<section id=sec8>
	<h1>Eight-level heading</h1>
	<section id=sec9>
	<h1>Ninth-level heading</h1>
	</section>
	</section>
	</section>
	</section>
	</section>
	</section>
	</section>
	</section>
EOF
echo >>$TMP1			# Add newline

cat >$TMP2 <<-EOF
<html><head><title>Test</title></head><body><h1 id="document-heading">Document heading</h1><!--begin-toc-->
<ul class="toc">
<li><a href="#document-heading">Document heading</a>
 <ul class="toc">
 <li><a href="#sec2">Second-level heading</a>
  <ul class="toc">
  <li><a href="#sec3">Third-level heading</a>
   <ul class="toc">
   <li><a href="#sec4">Fourth-level heading</a>
    <ul class="toc">
    <li><a href="#sec5">Fifth-level heading</a>
     <ul class="toc">
     <li><a href="#sec6">Sixth-level heading</a>
      <ul class="toc">
      <li><a href="#sec7">Seventh-level heading</a>
       <ul class="toc">
       <li><a href="#sec8">Eight-level heading</a>
       </ul>
      </ul>
     </ul>
    </ul>
   </ul>
  </ul>
 </ul></ul>
<!--end-toc--><section id="sec2">
<h1>Second-level heading</h1>
<section id="sec3">
<h1>Third-level heading</h1>
<section id="sec4">
<h1>Fourth-level heading</h1>
<section id="sec5">
<h1>Fifth-level heading</h1>
<section id="sec6">
<h1>Sixth-level heading</h1>
<section id="sec7">
<h1>Seventh-level heading</h1>
<section id="sec8">
<h1>Eight-level heading</h1>
<section id="sec9">
<h1>Ninth-level heading</h1>
</section>
</section>
</section>
</section>
</section>
</section>
</section>
</section></body></html>
EOF

diff -u $TMP2 $TMP1
