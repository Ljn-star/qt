param(
    [ValidateSet("code", "report")]
    [string]$Mode = "code"
)

$ErrorActionPreference = "Stop"
$repository = "Ljn-star/qt"
$root = Split-Path -Parent $PSScriptRoot
$apiRoot = "https://api.github.com/repos/$repository"

$credentialText = "protocol=https`nhost=github.com`n`n" | git credential fill 2>$null
$credential = @{}
foreach ($line in $credentialText) {
    $parts = $line -split "=", 2
    if ($parts.Count -eq 2) {
        $credential[$parts[0]] = $parts[1]
    }
}
if (-not $credential.ContainsKey("password")) {
    throw "No GitHub credential was found in Git Credential Manager."
}

$headers = @{
    "Accept" = "application/vnd.github+json"
    "Authorization" = "Bearer $($credential["password"])"
    "User-Agent" = "Codex"
    "X-GitHub-Api-Version" = "2022-11-28"
}

function Invoke-GitHub {
    param(
        [string]$Method,
        [string]$Uri,
        [object]$Body
    )
    $arguments = @{
        Method = $Method
        Uri = $Uri
        Headers = $headers
        TimeoutSec = 60
    }
    if ($null -ne $Body) {
        $arguments.ContentType = "application/json; charset=utf-8"
        $arguments.Body = $Body | ConvertTo-Json -Depth 20 -Compress
    }
    return Invoke-RestMethod @arguments
}

function New-Blob {
    param([string]$Path)
    $absolutePath = Join-Path $root $Path
    if (-not (Test-Path -LiteralPath $absolutePath -PathType Leaf)) {
        throw "Missing file: $Path"
    }
    $content = [Convert]::ToBase64String(
        [System.IO.File]::ReadAllBytes($absolutePath)
    )
    return Invoke-GitHub -Method "POST" -Uri "$apiRoot/git/blobs" -Body @{
        content = $content
        encoding = "base64"
    }
}

function Publish-Commit {
    param(
        [string]$Message,
        [string]$ParentSha,
        [string]$BaseTree,
        [string[]]$Paths
    )

    $treeEntries = @()
    foreach ($path in $Paths) {
        $blob = New-Blob -Path $path
        $treeEntries += @{
            path = $path.Replace("\", "/")
            mode = "100644"
            type = "blob"
            sha = $blob.sha
        }
        Write-Output "  uploaded $path"
    }

    $tree = Invoke-GitHub -Method "POST" -Uri "$apiRoot/git/trees" -Body @{
        base_tree = $BaseTree
        tree = $treeEntries
    }
    $timestamp = [DateTimeOffset]::UtcNow.ToString("yyyy-MM-ddTHH:mm:ssZ")
    $identity = @{
        name = "Ljn-star"
        email = "2811283369@qq.com"
        date = $timestamp
    }
    $commit = Invoke-GitHub -Method "POST" -Uri "$apiRoot/git/commits" -Body @{
        message = $Message
        tree = $tree.sha
        parents = @($ParentSha)
        author = $identity
        committer = $identity
    }
    Invoke-GitHub -Method "PATCH" -Uri "$apiRoot/git/refs/heads/master" -Body @{
        sha = $commit.sha
        force = $false
    } | Out-Null
    Write-Output "COMMIT $($commit.sha) $Message"
    return @{
        Sha = $commit.sha
        Tree = $tree.sha
    }
}

$reference = Invoke-GitHub -Method "GET" -Uri "$apiRoot/git/ref/heads/master"
$parentSha = $reference.object.sha
$parentCommit = Invoke-GitHub -Method "GET" -Uri "$apiRoot/git/commits/$parentSha"
$baseTree = $parentCommit.tree.sha

if ($Mode -eq "code") {
    $sourceExists = $false
    try {
        $existing = Invoke-GitHub `
            -Method "GET" `
            -Uri "$apiRoot/contents/HospitalManagementSystem/DatabaseManager.cpp?ref=master"
        $sourceExists = $null -ne $existing.sha
    } catch {
        if ($_.Exception.Response.StatusCode.value__ -ne 404) {
            throw
        }
    }
    if ($sourceExists) {
        throw "Experiment 3 source files already exist on the remote branch."
    }

    $groups = @(
        @{
            Message = "feat: add SQLite schema and patient table model"
            Paths = @(
                "HospitalManagementSystem/PatientData.h",
                "HospitalManagementSystem/DatabaseManager.h",
                "HospitalManagementSystem/DatabaseManager.cpp",
                "HospitalManagementSystem/PatientTableModel.h",
                "HospitalManagementSystem/PatientTableModel.cpp"
            )
        },
        @{
            Message = "feat: implement login and patient management UI"
            Paths = @(
                "HospitalManagementSystem/HospitalManagementSystem.pro",
                "HospitalManagementSystem/MainWindow.h",
                "HospitalManagementSystem/MainWindow.cpp",
                "HospitalManagementSystem/PatientEditorDialog.h",
                "HospitalManagementSystem/PatientEditorDialog.cpp",
                "HospitalManagementSystem/SexDelegate.h",
                "HospitalManagementSystem/SexDelegate.cpp",
                "HospitalManagementSystem/main.cpp",
                "HospitalManagementSystem/README.md"
            )
        },
        @{
            Message = "test: add Qt SQL tests and report screenshots"
            Paths = @(
                "HospitalManagementSystem/tests/HospitalTests.pro",
                "HospitalManagementSystem/tests/tst_hospital.cpp",
                "HospitalManagementSystem/HospitalCapture.pro",
                "HospitalManagementSystem/capture.cpp",
                "report_assets/00_schema.png",
                "report_assets/01_login.png",
                "report_assets/02_login_error.png",
                "report_assets/03_dashboard.png",
                "report_assets/04_patient_list.png",
                "report_assets/05_patient_editor.png",
                "report_assets/06_validation_error.png",
                "report_assets/07_after_add.png",
                "report_assets/08_delete_confirm.png",
                "report_assets/09_query_empty.png",
                "report_assets/10_history.png",
                "report_assets/qt-test-output.txt",
                "report_tools/build_report.py"
            )
        }
    )

    foreach ($group in $groups) {
        Write-Output "Creating: $($group.Message)"
        $result = Publish-Commit `
            -Message $group.Message `
            -ParentSha $parentSha `
            -BaseTree $baseTree `
            -Paths $group.Paths
        $parentSha = $result.Sha
        $baseTree = $result.Tree
    }
} else {
    $reportPath = "实验3-QtSQL数据库模块与View-Model结构的使用_2026秋_完成版.docx"
    $result = Publish-Commit `
        -Message "docs: publish completed experiment three report" `
        -ParentSha $parentSha `
        -BaseTree $baseTree `
        -Paths @(
            $reportPath,
            "report_tools/build_report.py",
            "report_tools/sync_github.ps1",
            "report_tools/update_report_git.py"
        )
    $parentSha = $result.Sha
}

Write-Output "FINAL_COMMIT=$parentSha"
Write-Output "REPOSITORY=https://github.com/$repository"
