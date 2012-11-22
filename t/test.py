# -*- coding: utf-8 -*-

import os, sys
sys.path.insert(0, os.path.abspath(os.path.dirname(os.path.dirname(__file__))))
import sanitize

passed = failed = 0

def test(testname, sanitizer, inp, expected):
    global passed, failed
    r = sanitizer.sanitize(inp)
    if r == expected:
        passed += 1
    else:
        failed += 1
        print "Test '%s' failed.\n  Input   : %s\n  Output  : %s\n  Expected: %s" % (testname, inp, r, expected)

if __name__ == '__main__':
    default_sanitizer    = sanitize.Sanitizer('modes/default.xml')
    basic_sanitizer      = sanitize.Sanitizer('modes/basic.xml')
    relaxed_sanitizer    = sanitize.Sanitizer('modes/relaxed.xml')
    restricted_sanitizer = sanitize.Sanitizer('modes/restricted.xml')

    # basic

    basic_html = "<b>Lo<!-- comment -->rem</b> <a href=\"pants\" title=\"foo\">ipsum</a> <a href=\"http://foo.com/\"><strong>dolor</strong></a> sit<br/>amet <script>alert(\"hello world\")</script>"

    test("basic-default", default_sanitizer, basic_html,
         "Lorem ipsum dolor sit amet alert(\"hello world\")")

    test("basic-restricted", restricted_sanitizer, basic_html,
         "<b>Lorem</b> ipsum <strong>dolor</strong> sit amet alert(\"hello world\")")

    test("basic-basic", basic_sanitizer, basic_html,
         "<b>Lorem</b> <a href=\"pants\">ipsum</a> <a href=\"http://foo.com/\"><strong>dolor</strong></a> sit<br>amet alert(\"hello world\")")

    test("basic-relaxed", relaxed_sanitizer, basic_html,
         "<b>Lorem</b> <a href=\"pants\" title=\"foo\">ipsum</a> <a href=\"http://foo.com/\"><strong>dolor</strong></a> sit<br>amet alert(\"hello world\")")

    # malformed

    malformed_html = "Lo<!-- comment -->rem</b> <a href=pants title=\"foo>ipsum <a href=\"http://foo.com/\"><strong>dolor</a></strong> sit<br/>amet <script>alert(\"hello world\")"

    test("malformed-default", default_sanitizer, malformed_html,
         "Lorem dolor sit amet alert(\"hello world\")")

    test("malformed-restricted", restricted_sanitizer, malformed_html,
         "Lorem <strong>dolor</strong> sit amet alert(\"hello world\")")

    test("malformed-basic", basic_sanitizer, malformed_html,
         "Lorem <a href=\"pants\"><strong>dolor</strong></a> sit<br>amet alert(\"hello world\")")

    test("malformed-relaxed", relaxed_sanitizer, malformed_html,
         "Lorem <a href=\"pants\" title=\"foo&gt;ipsum &lt;a href=\"><strong>dolor</strong></a> sit<br>amet alert(\"hello world\")")

    # unclosed

    unclosed_html = "<p>a</p><blockquote>b"

    test("unclosed-default", default_sanitizer, unclosed_html,
         " a  b ")

    test("unclosed-restricted", restricted_sanitizer, unclosed_html,
         " a  b ")

    test("unclosed-basic", basic_sanitizer, unclosed_html,
         "<p>a</p><blockquote>b</blockquote>")

    test("unclosed-relaxed", relaxed_sanitizer, unclosed_html,
         "<p>a</p><blockquote>b</blockquote>")

    # malicious

    malicious_html = "<b>Lo<!-- comment -->rem</b> <a href=\"javascript:pants\" title=\"foo\">ipsum</a> <a href=\"http://foo.com/\"><strong>dolor</strong></a> sit<br/>amet <<foo>script>alert(\"hello world\")</script>"

    test("malicious-default", default_sanitizer, malicious_html,
         "Lorem ipsum dolor sit amet script&gt;alert(\"hello world\")")

    test("malicious-restricted", restricted_sanitizer, malicious_html,
         "<b>Lorem</b> ipsum <strong>dolor</strong> sit amet script&gt;alert(\"hello world\")")

    test("malicious-basic", basic_sanitizer, malicious_html,
         "<b>Lorem</b> <a>ipsum</a> <a href=\"http://foo.com/\"><strong>dolor</strong></a> sit<br>amet script&gt;alert(\"hello world\")")

    test("malicious-relaxed", relaxed_sanitizer, malicious_html,
         "<b>Lorem</b> <a title=\"foo\">ipsum</a> <a href=\"http://foo.com/\"><strong>dolor</strong></a> sit<br>amet script&gt;alert(\"hello world\")")

    # raw-comment

    raw_comment_html = "<!-- comment -->Hello"
    
    test("raw-comment-default", default_sanitizer, raw_comment_html,
         "Hello")

    test("raw-comment-restricted", restricted_sanitizer, raw_comment_html,
         "Hello")

    test("raw-comment-basic", basic_sanitizer, raw_comment_html,
         "Hello")

    test("raw-comment-relaxed", relaxed_sanitizer, raw_comment_html,
         "Hello")

    # protocol-based JS injection: simple, no spaces

    js_injection_html_1 = "<a href=\"javascript:alert(\'XSS\')\">foo</a>"

    test("js-injection-1-default", default_sanitizer, js_injection_html_1,
         "foo")

    test("js-injection-1-restricted", restricted_sanitizer, js_injection_html_1,
         "foo")

    test("js-injection-1-basic", basic_sanitizer, js_injection_html_1,
         "<a>foo</a>")

    test("js-injection-1-relaxed", relaxed_sanitizer, js_injection_html_1,
         "<a>foo</a>")

    # protocol-based JS injection: simple, spaces before

    js_injection_html_2 = "<a href=\"javascript :alert(\'XSS\')\">foo</a>"

    test("js-injection-2-default", default_sanitizer, js_injection_html_2,
         "foo")

    test("js-injection-2-restricted", restricted_sanitizer, js_injection_html_2,
         "foo")

    test("js-injection-2-basic", basic_sanitizer, js_injection_html_2,
         "<a>foo</a>")

    test("js-injection-2-relaxed", relaxed_sanitizer, js_injection_html_2,
         "<a>foo</a>")

    # protocol-based JS injection: simple, spaces after

    js_injection_html_3 = "<a href=\"javascript: alert(\'XSS\')\">foo</a>"

    test("js-injection-3-default", default_sanitizer, js_injection_html_3,
         "foo")

    test("js-injection-3-restricted", restricted_sanitizer, js_injection_html_3,
         "foo")

    test("js-injection-3-basic", basic_sanitizer, js_injection_html_3,
         "<a>foo</a>")

    test("js-injection-3-relaxed", relaxed_sanitizer, js_injection_html_3,
         "<a>foo</a>")

    # protocol-based JS injection: simple, spaces before and after

    js_injection_html_4 = "<a href=\"javascript : alert(\'XSS\')\">foo</a>"

    test("js-injection-4-default", default_sanitizer, js_injection_html_4,
         "foo")

    test("js-injection-4-restricted", restricted_sanitizer, js_injection_html_4,
         "foo")

    test("js-injection-4-basic", basic_sanitizer, js_injection_html_4,
         "<a>foo</a>")

    test("js-injection-4-relaxed", relaxed_sanitizer, js_injection_html_4,
         "<a>foo</a>")

    # protocol-based JS injection: preceding colon

    js_injection_html_5 = "<a href=\":javascript:alert(\'XSS\')\">foo</a>"

    test("js-injection-5-default", default_sanitizer, js_injection_html_5,
         "foo")

    test("js-injection-5-restricted", restricted_sanitizer, js_injection_html_5,
         "foo")

    test("js-injection-5-basic",  basic_sanitizer, js_injection_html_5,
         "<a>foo</a>")

    test("js-injection-5-relaxed", relaxed_sanitizer, js_injection_html_5,
         "<a>foo</a>")

    # protocol-based JS injection: UTF-8 encoding
    
    js_injection_html_6 = "<a href=\"javascript&#58\">foo</a>"

    test("js-injection-6-default", default_sanitizer, js_injection_html_6,
         "foo")

    test("js-injection-6-restricted", restricted_sanitizer, js_injection_html_6,
         "foo")
    
    test("js-injection-6-basic", basic_sanitizer, js_injection_html_6,
         "<a>foo</a>")
    
    test("js-injection-6-relaxed", relaxed_sanitizer, js_injection_html_6,
         "<a>foo</a>")
    
    # protocol-based JS injection: long UTF-8 encoding

    js_injection_html_7 = "<a href=\"javascript&#0058\">foo</a>"
    
    test("js-injection-7-default", default_sanitizer, js_injection_html_7,
         "foo")

    test("js-injection-7-restricted", restricted_sanitizer, js_injection_html_7,
         "foo")

    test("js-injection-7-basic", basic_sanitizer, js_injection_html_7,
         "<a>foo</a>")

    test("js-injection-7-relaxed", relaxed_sanitizer, js_injection_html_7,
         "<a>foo</a>")

    # protocol-based JS injection: long UTF-8 encoding without semicolons

    js_injection_html_8 = "<a href=&#0000106&#0000097&#0000118&#0000097&#0000115&#0000099&#0000114&#0000105&#0000112&#0000116&#0000058&#0000097&#0000108&#0000101&#0000114&#0000116&#0000040&#0000039&#0000088&#0000083&#0000083&#0000039&#0000041>foo</a>"

    test("js-injection-8-default", default_sanitizer, js_injection_html_8,
         "foo")

    test("js-injection-8-restricted", restricted_sanitizer, js_injection_html_8,
         "foo")
   
    test("js-injection-8-basic", basic_sanitizer, js_injection_html_8,
         "<a>foo</a>")

    test("js-injection-8-relaxed", relaxed_sanitizer, js_injection_html_8,
         "<a>foo</a>")
    
    # protocol-based JS injection: hex encoding

    js_injection_html_9 = "<a href=\"javascript&#x3A\">foo</a>"
    
    test("js-injection-9-default", default_sanitizer, js_injection_html_9,
         "foo")

    test("js-injection-9-restricted", restricted_sanitizer, js_injection_html_9,
         "foo")

    test("js-injection-9-basic", basic_sanitizer, js_injection_html_9,
         "<a>foo</a>")
    
    test("js-injection-9-relaxed", relaxed_sanitizer, js_injection_html_9,
         "<a>foo</a>")

    # protocol-based JS injection: long hex encoding
    
    js_injection_html_10 = "<a href=\"javascript&#x003A\">foo</a>"

    test("js-injection-10-default", default_sanitizer, js_injection_html_10,
         "foo")
    
    test("js-injection-10-restricted", restricted_sanitizer, js_injection_html_10,
         "foo")
    
    test("js-injection-10-basic", basic_sanitizer, js_injection_html_10,
         "<a>foo</a>")
   
    test("js-injection-10-relaxed", relaxed_sanitizer, js_injection_html_10,
         "<a>foo</a>")

    # protocol-based JS injection: hex encoding without semicolons

    js_injection_html_11 = "<a href=&#x6A&#x61&#x76&#x61&#x73&#x63&#x72&#x69&#x70&#x74&#x3A&#x61&#x6C&#x65&#x72&#x74&#x28&#x27&#x58&#x53&#x53&#x27&#x29>foo</a>"

    test("js-injection-11-default", default_sanitizer, js_injection_html_11,
         "foo")
    
    test("js-injection-11-restricted", restricted_sanitizer, js_injection_html_11,
         "foo")
    
    test("js-injection-11-basic", basic_sanitizer, js_injection_html_11,
         "<a>foo</a>")
    
    test("js-injection-11-relaxed", relaxed_sanitizer, js_injection_html_11,
         "<a>foo</a>")

    # should translate valid HTML entities

    test("misc-1", default_sanitizer,
         "Don&apos;t tas&eacute; me &amp; bro!",
         u"Don't tasé me &amp; bro!")
    
    # should translate valid HTML entities while encoding unencoded ampersands

    test("misc-2", default_sanitizer,
         "cookies&sup2; & &frac14; cr&eacute;me",
         u"cookies² &amp; ¼ créme")

    # should never output &apos
    
    test("misc-3", default_sanitizer,
         "<a href='&apos' class=\"' &#39\">IE6 isn't a real browser</a>",
         "IE6 isn't a real browser")

    # should not choke on several instances of the same element in a row

    test("misc-4", default_sanitizer,
         "<img src=\"http://www.google.com/intl/en_ALL/images/logo.gif\"><img src=\"http://www.google.com/intl/en_ALL/images/logo.gif\"><img src=\"http://www.google.com/intl/en_ALL/images/logo.gif\"><img src=\"http://www.google.com/intl/en_ALL/images/logo.gif\">",
         "")

    # should surround the contents of :whitespace_elements with space characters when removing the element

    test("misc-5", default_sanitizer,
         "foo<div>bar</div>baz",
         "foo bar baz")

    test("misc-6", default_sanitizer,
         "foo<br>bar<br>baz",
         "foo bar baz")
    
    test("misc-7", default_sanitizer,
         "foo<hr>bar<hr>baz",
         "foo bar baz")

    # in-memory

    in_memory_sanitizer = sanitize.Sanitizer(data='''<?xml version="1.0" encoding="UTF-8"?>
        <mode>
            <elements>
                <b/>
                <span/>
            </elements>
            <delete>
                <script/>
                <style/>
            </delete>
            <rename>
                <br/>
            </rename>
            <rename to="span">
                <strong/>
            </rename>
        </mode>''')

    delete_html = "<b>Lo<!-- comment -->rem</b> <a href=\"javascript:pants\" title=\"foo\">ipsum</a> <a href=\"http://foo.com/\"><strong>dolor</strong></a> sit<br/>amet <script>alert(\"hello world\")</script>"

    test("delete", in_memory_sanitizer, delete_html,
         "<b>Lorem</b> ipsum <span>dolor</span> sit amet ")

    del default_sanitizer
    del basic_sanitizer
    del relaxed_sanitizer
    del restricted_sanitizer
    del in_memory_sanitizer

    total = passed + failed
    print u'Did %d checks.\n  Pass: %3d (%3d%%)\n  Fail: %3d (%3d%%)\n' % (
         total,
         passed,
         passed * 100 / total,
         failed,
         failed * 100 / total)

