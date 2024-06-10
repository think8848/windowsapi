<?php

header("Content-Type:text/html;charset=utf-8");
session_start();

if (isset($_POST['login'])) {
    $username = trim($_POST['username']);
    $password = trim($_POST['password']);
    if (($username == '') || ($password == '')) {
        // 用户名或密码不能为空
        header('refresh:3;url=login.html');
        echo "用户名或密码不能为空，3秒后跳转到登录页面";
        exit;
    } else if (($username != 'Admin') || ($password != '123456')) {
        // 用户名或密码错误 
        header('refresh:3;url=login.html');
        echo "用户名或密码错误，3秒后跳转到登录页面";
        exit;
    } else if (($username == 'Admin') && ($password == '123456')) {
        // 登录成功，将信息保存到session中 
        $_SESSION['username'] = $username;
        $_SESSION['islogin'] = 1;

        // 如果勾选7天内自动保存，则将其保存到cookie 
        if ($_POST['remember'] == "yes") {
            setcookie("username", $username, time() + 7 * 24 * 60 * 60);
            setcookie("password", md5($username . md5($password)), 
                    time() + 7 * 24 * 60 * 60);
        } else {
            setcookie("username", '', time() - 1);
            setcookie("password", '', time() - 1);
        }

        // 跳转到用户首页index.php
        header('refresh:3;url=index.php');
    }
} 