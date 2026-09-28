param(
    [string]$Root = (Join-Path $env:USERPROFILE 'Documents\Codex\CRTC2026')
)

# 注意：这里不用 Stop。以前用 Stop 时，git 往 stderr 写一句普通提示
# （比如"没有配置远端"）就会被 PowerShell 当成致命错误、直接把窗口关掉。
$ErrorActionPreference = 'Continue'
[Console]::OutputEncoding = [System.Text.Encoding]::UTF8

function Write-Step($text) { Write-Host "`n== $text" -ForegroundColor Cyan }
function Write-Ok($text) { Write-Host "   $text" -ForegroundColor Green }
function Write-Warn($text) { Write-Host "   $text" -ForegroundColor Yellow }
function Write-Err($text) { Write-Host "   $text" -ForegroundColor Red }

# 统一封装 git 调用：不抛异常，返回退出码和输出
function Invoke-Git {
    $output = & git -C $Root @args 2>&1
    return [pscustomobject]@{
        Code   = $LASTEXITCODE
        Output = @($output)
    }
}

function Get-GitText {
    $result = Invoke-Git @args
    if ($result.Code -ne 0) { return '' }
    $lines = @($result.Output | Where-Object { $_ -ne $null -and "$_" -ne '' })
    if ($lines.Count -eq 0) { return '' }
    return ("$($lines[0])").Trim()
}

Write-Host 'CRTC2026 文档上传助手' -ForegroundColor Cyan
Write-Host '（把本机文档同步到 GitHub / Gitee）'

try {
    if (-not (Test-Path -LiteralPath $Root)) {
        Write-Err "找不到项目目录：$Root"
        Read-Host '按回车退出'
        exit 1
    }
    if (-not (Get-Command git -ErrorAction SilentlyContinue)) {
        Write-Err '没有检测到 Git，请先安装 Git for Windows。'
        Read-Host '按回车退出'
        exit 1
    }

    # 1) 本地仓库与身份
    Write-Step '检查本地仓库与提交身份'
    if (-not (Test-Path -LiteralPath (Join-Path $Root '.git'))) {
        $init = Invoke-Git init -b main
        if ($init.Code -eq 0) { Write-Ok '已初始化本地仓库（分支 main）' }
        else { Write-Warn '初始化仓库时出现提示，继续尝试' }
    } else {
        Write-Ok '本地仓库已存在'
    }

    # git 的安全检查：如果仓库是别的账户（比如我这边）创建的，
    # git 会以"可疑所有权"为由拒绝一切操作，这里自动加例外。
    $probe = Invoke-Git status
    $probeText = ($probe.Output -join "`n")
    if ($probeText -match 'dubious ownership') {
        Write-Warn '检测到 git 的"可疑所有权"保护（这个仓库是别的账户创建的），正在自动添加例外...'
        $safePath = ($Root -replace '\\', '/')
        Invoke-Git config --global --add safe.directory $safePath | Out-Null
        $probe = Invoke-Git status
        $probeText = ($probe.Output -join "`n")
        if ($probeText -match 'dubious ownership') {
            Write-Err '自动添加例外没成功。解决办法：在项目文件夹里删掉隐藏的 .git 文件夹，再重新运行本助手。'
            Write-Err '（删 .git 只影响本地提交历史，文档内容一点都不会丢）'
            Read-Host '按回车退出'
            exit 1
        }
        Write-Ok '已添加例外，继续'
    }

    $name = Get-GitText config user.name
    if (-not $name) {
        $name = Read-Host '请输入你的名字（会写进提交记录，例如 蔡昌恒）'
        $name = ("$name").Trim()
    }
    $email = Get-GitText config user.email
    if (-not $email) {
        $email = Read-Host '请输入你的邮箱（注册 GitHub/Gitee 用的那个）'
        $email = ("$email").Trim()
    }

    if (-not $name -or -not $email) {
        Write-Err '名字或邮箱没有填（直接回车了？），本次已取消，没有做任何修改。'
        Read-Host '按回车退出'
        exit 1
    }

    # 写入配置：先试本仓库，不行就写全局；两者都失败也没关系，
    # 因为下面提交时会用 -c 显式带上身份，不依赖配置文件。
    Invoke-Git config user.name $name | Out-Null
    Invoke-Git config user.email $email | Out-Null
    if ((Get-GitText config user.name) -ne $name) {
        Invoke-Git config --global user.name $name | Out-Null
        Invoke-Git config --global user.email $email | Out-Null
        if ((Get-GitText config user.name) -eq $name) {
            Write-Warn '本仓库配置文件写不进去，已改用全局配置（本机所有仓库通用）'
        } else {
            Write-Warn '配置文件写入没成功，但提交时会直接带上身份信息，不影响上传'
        }
    }
    Write-Ok "提交身份：$name <$email>"

    # 2) 远端仓库
    Write-Step '检查远端仓库地址'
    $remote = Get-GitText remote get-url origin
    if (-not $remote) {
        Write-Warn '还没有配置远端仓库。请先在 GitHub 或 Gitee 上新建一个【公开】空仓库，'
        Write-Warn '名字建议用队名拼音：shangjianyinqing（建仓库时不要勾 README）'
        Write-Host ''
        $input_url = Read-Host '把仓库地址粘贴到这里（形如 https://github.com/用户名/shangjianyinqing.git）'
        $input_url = ("$input_url").Trim()
        if (-not $input_url) {
            Write-Warn '没有填地址，本次已取消（没有做任何修改）。'
            Write-Host ''
            Read-Host '按回车退出'
            exit 0
        }
        Invoke-Git remote add origin $input_url | Out-Null
        $remote = Get-GitText remote get-url origin
        if (-not $remote) {
            Write-Err '仓库地址没设置成功，请检查地址是否完整（建议直接复制 GitHub 页面上的地址）。'
            Read-Host '按回车退出'
            exit 1
        }
    }
    Write-Ok "远端：$remote"

    # 3) 提交
    Write-Step '提交本机改动'
    Invoke-Git add -A | Out-Null
    $status = Invoke-Git status --porcelain
    $pending = @($status.Output | Where-Object { "$_" -ne '' })
    if ($pending.Count -gt 0) {
        $message = "更新文档：$($pending.Count) 个文件（" + (Get-Date -Format 'yyyy-MM-dd HH:mm') + "）"
        $cn = "user.name=$name"
        $ce = "user.email=$email"
        $commit = Invoke-Git -c $cn -c $ce commit -m $message
        if ($commit.Code -eq 0) {
            Write-Ok "已提交 $($pending.Count) 个文件的改动"
        } else {
            Write-Warn '提交时出现提示：'
            $commit.Output | ForEach-Object { Write-Host "     $_" }
        }
    } else {
        Write-Ok '没有新改动，跳过提交'
    }

    # 4) 推送
    Write-Step '推送到远端'

    # 关键一步：有些仓库的默认分支叫 master，而远端一般用 main，
    # 不统一就会出现 "src refspec main does not match any" 这种错。
    $hasCommit = (Invoke-Git rev-parse --verify HEAD).Code -eq 0
    if ($hasCommit) {
        $branch = Get-GitText rev-parse --abbrev-ref HEAD
        if ($branch -and $branch -ne 'main') {
            $rename = Invoke-Git branch -M main
            if ($rename.Code -eq 0) {
                Write-Ok "本地分支已改名为 main（原来是 $branch）"
            } else {
                Write-Warn "分支改名失败（原来叫 $branch），继续尝试推送"
            }
        } else {
            Write-Ok "本地分支：main"
        }
    } else {
        Write-Warn '本地还没有提交（仓库是空的），先跳过分支检查'
    }

    Write-Warn '第一次推送会弹出登录窗口，请选「Sign in with your browser / 用浏览器登录」'
    $push = Invoke-Git push -u origin main
    $push.Output | ForEach-Object { Write-Host "     $_" }
    if ($push.Code -ne 0) {
        # 最常见的情况：远端仓库不是空的（建仓库时勾了 README），先自动合并再推
        $remoteHeads = Invoke-Git ls-remote --heads origin main
        $hasRemoteMain = @($remoteHeads.Output | Where-Object { "$_" -match 'refs/heads/main' }).Count -gt 0
        if ($hasRemoteMain) {
            Write-Host ''
            Write-Warn '推送被拒绝，正在尝试自动合并远端已有内容（例如建仓库时勾了 README）...'
            $pull = Invoke-Git pull origin main --allow-unrelated-histories --no-edit
            $pull.Output | ForEach-Object { Write-Host "     $_" }
            if ($pull.Code -eq 0) {
                $push = Invoke-Git push -u origin main
                $push.Output | ForEach-Object { Write-Host "     $_" }
            }
        } else {
            Write-Host ''
            Write-Warn '远端仓库还是空的（没有 main 分支），无需合并，直接重试推送...'
            $push = Invoke-Git push -u origin main
            $push.Output | ForEach-Object { Write-Host "     $_" }
        }
    }
    if ($push.Code -ne 0) {
        Write-Host ''
        Write-Err '推送失败。常见原因和处理办法：'
        Write-Host '   1. 没登录 / 令牌过期：GitHub 现在必须用浏览器登录或 Personal Access Token'
        Write-Host '   2. 网络不通（国内访问 GitHub 常不稳定）：改用 Gitee，或给 Git 设置代理'
        Write-Host '   3. 文件过大被拒：双击「检查体积.bat」看有没有超 100 MB 的文件'
        Write-Host '   4. 地址填错（末尾应是 .git）：把地址复制到浏览器里试试能不能打开'
    } else {
        $web = $remote -replace '\.git$', ''
        Write-Host ''
        Write-Ok '上传成功！仓库地址：'
        Write-Host "   $web" -ForegroundColor Green
    }
} catch {
    Write-Host ''
    Write-Err ('助手遇到意外错误：' + $_.Exception.Message)
    Write-Host '   把上面这段内容截图发给 Codex 即可。'
}

Write-Host ''
Read-Host '按回车关闭窗口'
