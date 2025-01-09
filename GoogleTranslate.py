#!/usr/bin/python3
import re
import html
from urllib import parse
import requests
import sys

GOOGLE_TRANSLATE_URL = 'http://translate.google.com/m?q=%s&tl=%s&sl=%s'

def translate(text, to_language="auto", text_language="auto"):

    text = parse.quote(text)
    url = GOOGLE_TRANSLATE_URL % (text,to_language,text_language)
    response = requests.get(url)
    data = response.text
    expr = r'(?s)class="(?:t0|result-container)">(.*?)<'
    result = re.findall(expr, data)
    if (len(result) == 0):
        return ""

    return html.unescape(result[0])

# print(translate("你吃饭了么?", "en","zh-CN")) #汉语转英语
# print(translate("你吃饭了么？", "ja","zh-CN")) #汉语转日语
# print(translate("about your situation", "zh-CN","en")) #英语转汉语

def main():
    args = sys.argv
    if len(args) > 1:
        print(translate(args[1], "zh-CN","en"))

if __name__ == "__main__":
    main()