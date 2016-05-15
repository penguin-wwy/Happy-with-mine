import Login
import re
#from collections import OrderedDict

def _title_info(title_link):
    pattern = re.compile(r'[><]?')
    m = re.split(pattern, title_link)
    return m

def _url_info(title_link):
    pattern = re.compile(r'/question/\d+')
    m = re.search(pattern, title_link)
    url = 'https://www.zhihu.com' + m.group()
    return url

def _look_num(question_link, index_page):
    pattern = re.compile(r'被浏览.*次')
    m = pattern.findall(index_page.text)
    p = re.compile(r'[><]?')
    g = re.split(p, m[0])
    return g

def _answer_num(question_link, index_page):
    pattern = re.compile(r'>.*个回答<')
    m = pattern.findall(index_page.text)
    g = re.search(r'\d+', m[0])
    return g.group()

def _question_info(title_link, file):
    session = Login.session

    #保存问题数据的字典
    question_result = {}

    #首先保存问题标题
    question_result['标题'] = _title_info(title_link)[4]

    #构造问题的url
    question_result['链接'] = _url_info(title_link)

    #获取问题链接页面
    index_page = session.get(question_result['链接'], headers=Login.headers)

    #浏览问题人数
    question_result['浏览次数'] = _look_num(question_result['链接'], index_page)[2]

    #回答数量
    question_result['回答数量'] = _answer_num(question_result['链接'], index_page)
    """
    file = open('C:\\Users\\penguin\\Desktop\\a.txt', 'wb+')
    file.write(index_page.content)
    file.close()

    file = open('C:\\Users\\penguin\\Desktop\\a.txt', 'r+', encoding= 'utf-8')
    """
    txt1 = '标题'
    txt2 = '链接'
    txt3 = '浏览次数'
    txt4 = '回答数量'
    file.write(txt1 + '  ' + question_result[txt1] + '\n'\
               + txt2 + '  ' + question_result[txt2] + '\n'\
               + txt3 + '  ' + question_result[txt3] + '\n'\
               + txt4 + '  ' + question_result[txt4] + '\n')
    #获取答主信息
    _answer_info(index_page, file)
    file.write('***************************************************************')
    file.write('\n')
    file.write('                            分割线                             ')
    file.write('\n')
    file.write('***************************************************************')
    file.write('\n')

    return question_result

def _answer_info(question_index_page, file):
    session = Login.session

    #结果保存字典
    #result = {}

    answer_pattern = re.compile(r'<a class="author-link"\ndata-tip=".*"\ntarget="_blank" href=".*"\n>.*</a>')
    answer_m = answer_pattern.findall(question_index_page.text)

    praise_pattern = re.compile(r'<span class="count">.*</span>')
    praise_m = praise_pattern.findall(question_index_page.text)

    count = len(answer_m)
    if count > len(praise_m):
        count = len(praise_m)
    i = 0

    while i < count:
        sub_str = re.split(r'[><]?', answer_m[i])
        #print(sub_str)
        #['', 'a class="author-link" data-tip="p$t$wei-yiyi" target="_blank" href="/people/wei-yiyi"', '韦一一', '/a', '']

        _sub_str = re.split(r'"?', sub_str[1])
        #print(_sub_str)
        #['a class=', 'author-link', ' data-tip=', 'p$t$wei-yiyi', ' target=', '_blank', ' href=', '/people/wei-yiyi', '']

        user_url = "https://www.zhihu.com" + _sub_str[7]
        index_page = session.get(user_url, headers=Login.headers)
        patter = re.compile(r'<span class="zg-gray-normal">关注者</span><br />\n<strong>\d*</strong>')
        m = patter.findall(index_page.text)
        concern_num = re.split(r'[><]?', m[0])[8]
        #['', 'span class="zg-gray-normal"', '关注者', '/span', '', 'br /', '\n', 'strong', '19', '/strong', '']

        que_pattern = re.compile(r'提问\n<span class="num">\d</span>\n</a>')
        que_text = re.search(que_pattern, index_page.text)
        if que_text != None:
            que_m = re.search(r'[0-9]+', que_text.group())
            _que_m = que_m.group()
        else:
            _que_m = ''
        #print(que_m.group())

        ask_pattern = re.compile(r'href=".*answers">\n回答\n<span class="num">[1234567890]+</span>\n</a>')
        ask_text = re.search(ask_pattern, index_page.text)
        if ask_text != None:
            ask_m = re.search(r'[0-9]+', ask_text.group())
            _ask_m = ask_m.group()
        else:
            _ask_m = ''
        #print(ask_m.group())

        praise_all_pattern = re.compile(r'<span class="zm-profile-header-user-agree"><span class="zm-profile-header-icon">.*</span>')
        praise_all_text = re.search(praise_all_pattern, index_page.text)
        if praise_all_text != None:
            praise_all_m = re.search(r'[0-9]+', praise_all_text.group())
            _praise_all_m = praise_all_m.group()
        else:
            _praise_all_m = ''

        greate_all_pattern = re.compile(r'<span class="zm-profile-header-user-thanks"><span class="zm-profile-header-icon">.*</span>')
        greate_all_text = re.search(greate_all_pattern, index_page.text)
        if greate_all_text != None:
            greate_all_m = re.search(r'[0-9]+', greate_all_text.group())
            _greate_all_m = greate_all_m.group()
        else:
            _greate_all_m = ''

        #file.write("答主：" + sub_str[2] + "  本答案点赞数：" + re.search(r'\d+', praise_m[i]).group()\
        #        + " 链接：" + user_url + "  关注人数：" + concern_num)
        file.write("链接：" + user_url + ",  答主：" + sub_str[2]\
                + ",  本答案点赞数：" + re.search(r'\d+', praise_m[i]).group()\
                + ",  关注人数：" + concern_num\
                + ",  提问次数：" + _que_m\
                + ",  回答次数："  + _ask_m\
                + ",  总赞数：" + _praise_all_m\
                + "， 总感谢数：" + _greate_all_m)
        file.write('\n')
        #print("答主:" + answer_m[i] + "  本答案点赞数:" + re.search(r'\d+', praise_m[i]).group())
        i += 1

def answer_info(read):
    session = Login.session

    #结果保存字典
    result = {}

    answer_pattern = re.compile(r'<a class="author-link" data-tip=".*" target="_blank" href=".*">.*</a>')
    answer_m = answer_pattern.findall(read)

    praise_pattern = re.compile(r'<span class="count">.*</span>')
    praise_m = praise_pattern.findall(read)

    f = open('C:\\Users\\penguin\\Desktop\\b.txt', 'w+', encoding='utf-8')

    count = len(answer_m)
    if count > len(praise_m):
        count = len(praise_m)
    i = 0
    while i < count:
        sub_str = re.split(r'[><]?', answer_m[i])
        #print(sub_str)
        #['', 'a class="author-link" data-tip="p$t$wei-yiyi" target="_blank" href="/people/wei-yiyi"', '韦一一', '/a', '']
        _sub_str = re.split(r'"?', sub_str[1])
        #print(_sub_str)
        #['a class=', 'author-link', ' data-tip=', 'p$t$wei-yiyi', ' target=', '_blank', ' href=', '/people/wei-yiyi', '']
        user_url = "https://www.zhihu.com" + _sub_str[7]
        index_page = session.get(user_url, headers=Login.headers)
        patter = re.compile(r'<span class="zg-gray-normal">关注者</span><br />\n<strong>\d*</strong>')
        m = patter.findall(index_page.text)
        concern_num = re.split(r'[><]?', m[0])[8]
        #['', 'span class="zg-gray-normal"', '关注者', '/span', '', 'br /', '\n', 'strong', '19', '/strong', '']
        f.write("答主：" + sub_str[2] + "  本答案点赞数：" + re.search(r'\d+', praise_m[i]).group()\
                + " 链接：" + user_url + "  关注人数：" + concern_num)
        f.write('\n')
        #print("答主:" + answer_m[i] + "  本答案点赞数:" + re.search(r'\d+', praise_m[i]).group())
        i += 1

    f.close()
"""
if __name__ == '__main__':
    f = open('C:\\Users\\penguin\\Desktop\\a.txt', 'r+', encoding='utf-8')
    answer_info(f.read())
"""