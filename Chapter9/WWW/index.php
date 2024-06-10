<?php

header("Content-Type:text/html;charset=utf-8");
session_start();

// 首先判断cookie是否有记住用户信息 
if (isset($_COOKIE['username'])) {
    $_SESSION['username'] = $_COOKIE['username'];
    $_SESSION['islogin'] = 1;
}

if (isset($_SESSION['islogin'])) {
    // 已经登录 
    echo $_SESSION['username'] . "：你好，欢迎进入个人中心！<br/>";
    echo "<a href='logout.php'>注销</a>";
} else {
    // 尚未登录 
    echo "你还未登录，请<a href='login.html'>登录</a>";
}