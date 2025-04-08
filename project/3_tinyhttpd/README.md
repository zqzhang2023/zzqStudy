# tinyhttpd 项目

写在前面：这个项目是  J. David 大佬 在1999年做出来的，虽然很老旧, 而且手打一遍之后也感觉有些地方可能不适合这么写，但是做完这个项目之后学到的东西有很多很多，如果大家有时间的话，不妨跟着做一做，相信一定会有很大的收获

膜拜大佬！！！

GIT 原始项目地址：https://github.com/EZLippi/Tinyhttpd

我这边加上注释的代码：

# 运行起来

做之前需要先确定自己的环境能够正常把项目运行起来，要不然做的过程之中不知道是自己代码的问题还是环境的问题

我用的Linux ubuntu 提前安装了perl 地址在 /usr/bin/perl(这个可以通过 which perl 命令得到)

GIT 原始项目地址：https://github.com/EZLippi/Tinyhttpd

1.克隆下来，解压 这个不必对多说

2.执行make命令
- 会生成 httpd 与 client (会有一些警告，不必在意)

3.执行httpd(./httpd)
- 这个时候浏览器输入 http://127.0.0.1:4000 会发现 没有东西
- 这需要修改资源文件(htdocs里面的CGI文件)

4.修改资源文件
- 两个CGI文件的路径 perl 
  - 两个文件的第一行都有 #!/usr/local/bin/perl -Tw 把这改成咱们自己的，我的是 /usr/bin/perl
- 修改资源文件权限
  - 修改一下 index.html 的权限
  - sudo chmod 600 index.html
  - 要为 check.cgi color.cgi 添加可执行的权限
  - sudo chmod +x check.cgi
  - sudo chmod +x color.cgi

5.正常运行起来

![alt text](images/1_运行起来.png)

![alt text](images/2_提交请求.png)

# 自己做一遍

无论怎么说，还是得自己对着做一遍的



