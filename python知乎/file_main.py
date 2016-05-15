import Login
import Info
import requests
import http
import re
import time
import os

if __name__ == '__main__':
    if Login.isLogin():
        print("已登录")
    else:
        account = input("请输入你的用户名\n>  ")
        secret = input("请输入你的密码\n>  ")
        Login.login(secret, account)

    index_page = Login.session.get('https://www.zhihu.com/#signin', headers=Login.headers) #主页
    #index_page = Login.session.get('https://www.zhihu.com/explore', headers=Login.headers)  #发现
    """
    f = open('C:\\Users\\penguin\\Desktop\\dump.txt', 'wb+')
    f.write(index_page.content)
    f.close()
    file = open('C:\\Users\\penguin\\Desktop\\dump.txt', 'r+', encoding= 'utf-8')
    """
    file_name = str(time.localtime()[0]) + '.' + str(time.localtime()[1]) + '.' + str(time.localtime()[2])\
    + '.' + str(time.localtime()[3]) + '.' + str(time.localtime()[4]) + '.' + str(time.localtime()[5])
    file_name += '_result.txt'
    path = os.getcwd() + '\\' + file_name
    file = open(path, 'w+', encoding='utf-8')
    p = re.compile(r'<h2 class=".*"><a class=".*" target=".*" href=".*">.*</a></h2>')
    for link in p.findall(index_page.text):
        if link != None:
            question_result = Info._question_info(link, file)
    file.close()