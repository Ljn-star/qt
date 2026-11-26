from __future__ import annotations

import copy
import shutil
from pathlib import Path

from docx import Document
from docx.enum.text import WD_ALIGN_PARAGRAPH
from docx.oxml import OxmlElement
from docx.oxml.ns import qn
from docx.shared import Inches, Pt, RGBColor
from docx.text.paragraph import Paragraph


ROOT = Path(__file__).resolve().parents[1]
SOURCE = Path(
    r"C:\Users\Lenovo\Desktop\实验3-QtSQL数据库模块与View-Model结构的使用_2026秋.docx"
)
OUTPUT = ROOT / "实验3-QtSQL数据库模块与View-Model结构的使用_2026秋_完成版.docx"
ASSETS = ROOT / "report_assets"


def add_paragraph_after(
    anchor: Paragraph,
    text: str = "",
    *,
    font_name: str = "宋体",
    size: float = 12.0,
    bold: bool = False,
    color: str = "000000",
    left_indent: float = 0.68,
    line_spacing: float = 1.5,
    space_after: float = 2.0,
    alignment: WD_ALIGN_PARAGRAPH | None = None,
) -> Paragraph:
    paragraph_element = OxmlElement("w:p")
    anchor._p.addnext(paragraph_element)
    paragraph = Paragraph(paragraph_element, anchor._parent)
    paragraph.style = "Normal"
    paragraph.paragraph_format.left_indent = Inches(left_indent)
    paragraph.paragraph_format.first_line_indent = Inches(0)
    paragraph.paragraph_format.line_spacing = line_spacing
    paragraph.paragraph_format.space_after = Pt(space_after)
    if alignment is not None:
        paragraph.alignment = alignment

    if text:
        run = paragraph.add_run(text)
        run.bold = bold
        run.font.name = font_name
        run.font.size = Pt(size)
        run.font.color.rgb = RGBColor.from_string(color)
        run._element.get_or_add_rPr().rFonts.set(qn("w:eastAsia"), font_name)
    return paragraph


def add_runs_after(
    anchor: Paragraph,
    runs: list[tuple[str, bool]],
    *,
    font_name: str = "宋体",
    size: float = 12.0,
    left_indent: float = 0.68,
) -> Paragraph:
    paragraph = add_paragraph_after(
        anchor,
        "",
        font_name=font_name,
        size=size,
        left_indent=left_indent,
    )
    for text, bold in runs:
        run = paragraph.add_run(text)
        run.bold = bold
        run.font.name = font_name
        run.font.size = Pt(size)
        run.font.color.rgb = RGBColor(0, 0, 0)
        run._element.get_or_add_rPr().rFonts.set(qn("w:eastAsia"), font_name)
    return paragraph


def add_code_after(anchor: Paragraph, code: str) -> Paragraph:
    paragraph = add_paragraph_after(
        anchor,
        "",
        font_name="Consolas",
        size=8.5,
        left_indent=0.82,
        line_spacing=1.0,
        space_after=5.0,
    )
    lines = code.strip("\n").splitlines()
    for index, line in enumerate(lines):
        if index:
            paragraph.add_run().add_break()
        run = paragraph.add_run(line)
        run.font.name = "Consolas"
        run.font.size = Pt(8.5)
        run.font.color.rgb = RGBColor(44, 62, 80)
        run._element.get_or_add_rPr().rFonts.set(qn("w:eastAsia"), "等线")
    return paragraph


def add_image_after(
    anchor: Paragraph,
    image_name: str,
    caption: str,
    width: float = 6.15,
) -> Paragraph:
    picture_paragraph = add_paragraph_after(
        anchor,
        "",
        left_indent=0.0,
        line_spacing=1.0,
        space_after=2.0,
        alignment=WD_ALIGN_PARAGRAPH.CENTER,
    )
    picture_paragraph.add_run().add_picture(
        str(ASSETS / image_name), width=Inches(width)
    )
    caption_paragraph = add_paragraph_after(
        picture_paragraph,
        caption,
        size=9.5,
        left_indent=0.0,
        line_spacing=1.0,
        space_after=7.0,
        alignment=WD_ALIGN_PARAGRAPH.CENTER,
    )
    for run in caption_paragraph.runs:
        run.font.color.rgb = RGBColor(55, 70, 85)
    return caption_paragraph


def add_block(
    anchor: Paragraph,
    body_paragraphs: list[str],
    code_blocks: list[str] | None = None,
    images: list[tuple[str, str, float]] | None = None,
) -> Paragraph:
    current = anchor
    for text in body_paragraphs:
        current = add_paragraph_after(current, text)
    for code in code_blocks or []:
        current = add_paragraph_after(
            current,
            "主要代码：",
            font_name="黑体",
            size=10.5,
            bold=True,
            left_indent=0.68,
        )
        current = add_code_after(current, code)
    for image_name, caption, width in images or []:
        current = add_image_after(current, image_name, caption, width)
    return current


def main() -> None:
    shutil.copy2(SOURCE, OUTPUT)
    document = Document(OUTPUT)
    cell = document.tables[0].cell(4, 0)

    anchors = {
        "database": cell.paragraphs[36],
        "ui": cell.paragraphs[38],
        "switch": cell.paragraphs[40],
        "db_support": cell.paragraphs[42],
        "login": cell.paragraphs[44],
        "list_delete": cell.paragraphs[46],
        "add_edit": cell.paragraphs[48],
        "git": cell.paragraphs[50],
        "tests": cell.paragraphs[52],
        "summary": cell.paragraphs[54],
    }
    appendix_paragraphs = cell.paragraphs[55:75]

    add_block(
        anchors["database"],
        [
            "本实验使用 Qt SQL 的 QSQLITE 驱动建立 hospital.sqlite 数据库。Patient 表保存患者档案，"
            "User 表保存登录信息，Department 与 Doctor 两表通过 DEPARTMENT_ID 外键关联，History 表记录"
            "登录和患者维护操作。ID_CARD 设置唯一约束，SEX、HEIGHT、WEIGHT、AGE 设置 CHECK 约束，"
            "使界面校验与数据库约束共同保护数据完整性。",
            "数据库在程序首次运行时自动建表，并写入管理员、科室、医生和两名演示患者。"
            "Navicat 与 SQLite 均可实施同类表结构，本程序采用无需额外服务器配置的 QSQLITE，"
            "便于在 Qt Creator 中直接复现。",
        ],
        [
            """
QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
db.setDatabaseName("hospital.sqlite");
db.open();

QSqlQuery query(db);
query.exec("PRAGMA foreign_keys = ON");
// Patient(ID, ID_CARD UNIQUE, NAME, SEX, DOB, HEIGHT,
//         WEIGHT, MOBILEPHONE, AGE, CREATEDTIMESTAMP)
            """,
        ],
        [
            (
                "00_schema.png",
                "图 1  运行时数据库表结构核验（依据实际 SQLite 元数据）",
                6.05,
            )
        ],
    )

    add_block(
        anchors["ui"],
        [
            "界面采用 QMainWindow 与 QStackedWidget 组织。登录页由品牌展示区和登录卡片组成，"
            "登录后的工具栏包含工作台、患者管理、医生信息、科室信息、操作记录和退出登录入口；"
            "工具栏控件通过代码创建，开发者姓名与学号固定显示在登录页和登录后的工具栏中。",
        ],
        [
            """
MainWindow::MainWindow(const QString &databasePath, QWidget *parent)
    : QMainWindow(parent)
{
    m_stack = new QStackedWidget(this);
    setCentralWidget(m_stack);
    m_stack->addWidget(buildLoginPage());
    buildToolbar();
    openDatabase(databaseFile(databasePath));
    buildAuthenticatedPages();
}
            """,
        ],
        [
            (
                "01_login.png",
                "图 2  初始登录界面与开发者信息",
                6.15,
            )
        ],
    )

    add_block(
        anchors["switch"],
        [
            "QStackedWidget 中各页索引依次为登录页、工作台、患者管理、医生信息、科室信息和操作记录。"
            "工具栏 QAction 和工作台卡片调用统一导航方法切换页面，同时更新导航按钮的选中状态，"
            "避免多个入口造成页面状态不一致。",
        ],
        [
            """
void MainWindow::navigateToPatients()
{
    m_stack->setCurrentIndex(2);
    m_patientAction->setChecked(true);
    m_doctorAction->setChecked(false);
    m_departmentAction->setChecked(false);
    m_historyAction->setChecked(false);
    m_patientModel->refresh();
    updatePatientStatus();
}
            """,
        ],
        [
            (
                "03_dashboard.png",
                "图 3  登录成功后的工作台与代码化工具栏",
                6.15,
            )
        ],
    )

    add_block(
        anchors["db_support"],
        [
            "DatabaseManager 以单例封装数据库连接、建表、演示数据和登录查询。连接失败时，"
            "错误文本会显示在登录页并禁用登录按钮；正常连接后执行 PRAGMA foreign_keys=ON。"
            "患者列表由 QSqlTableModel 直接绑定 QTableView，修改策略设为 OnManualSubmit，"
            "所有新增、修改和删除均显式提交并重新 select，确保界面与数据库同步。",
        ],
        [
            """
PatientTableModel::PatientTableModel(QObject *parent)
    : QSqlTableModel(parent, DatabaseManager::instance().database())
{
    setTable("Patient");
    setEditStrategy(QSqlTableModel::OnManualSubmit);
    setSort(CreatedAtColumn, Qt::DescendingOrder);
    select();
}

if (!submitAll()) {
    error = "保存患者失败：" + lastError().text();
    revertAll();
}
select();
            """,
        ],
        [
            (
                "04_patient_list.png",
                "图 4  绑定数据库后的患者列表",
                6.15,
            )
        ],
    )

    add_block(
        anchors["login"],
        [
            "登录使用 QSqlQuery 预处理语句查询 User 表。密码先经 QCryptographicHash 的 SHA-256 摘要，"
            "再参与查询；用户名或密码错误、输入为空、数据库未连接分别显示不同提示。登录成功后写入"
            "History 表并进入工作台，退出登录时同步记录。",
        ],
        [
            """
query.prepare(
    "SELECT ID, FULLNAME FROM User "
    "WHERE USERNAME = :username AND PASSWORD = :password");
query.bindValue(":username", username.trimmed());
query.bindValue(":password", hashPassword(password));

if (!query.exec()) { /* 显示数据库查询错误 */ }
if (!query.next()) { error = "用户名或密码错误"; return false; }
            """,
        ],
        [
            (
                "02_login_error.png",
                "图 5  错误凭据的登录校验结果",
                6.25,
            )
        ],
    )

    add_block(
        anchors["list_delete"],
        [
            "患者列表支持按姓名、身份证号或手机号模糊查询。查询前对单引号进行转义，避免错误拼接；"
            "没有匹配记录时保留空表并显示明确状态。删除前弹出确认框，用户确认后调用 removeRow、"
            "submitAll 和 select，数据库与视图同步刷新；取消删除不会改变任何记录。",
            "性别列通过自定义 SexDelegate 提供“男/女”下拉框修改；列表还显示自动计算的年龄、UUID、"
            "创建时间，以及本次操作的记录入口，满足 View/Model 与操作留痕要求。",
        ],
        [
            """
escaped.replace('\'', "''");
setFilter("NAME LIKE '%keyword%' OR "
          "ID_CARD LIKE '%keyword%' OR "
          "MOBILEPHONE LIKE '%keyword%'");
select();

removeRow(row);
submitAll();
select();
            """,
        ],
        [
            (
                "09_query_empty.png",
                "图 6  查询无结果时的界面状态",
                6.15,
            ),
            (
                "08_delete_confirm.png",
                "图 7  删除前的二次确认",
                4.55,
            ),
        ],
    )

    add_block(
        anchors["add_edit"],
        [
            "新增和编辑共用 PatientEditorDialog。界面校验身份证号、姓名、手机号、身高、体重和出生日期，"
            "年龄由出生日期自动计算；新增时由 QUuid 生成唯一 ID，由 QDateTime 写入创建时间。"
            "身份证号重复由数据库唯一约束再次拦截，提交失败时回滚未提交的模型修改并提示原因。",
            "修改时保留原 UUID 与创建时间，只更新业务字段。新增或修改成功后立即刷新模型，"
            "同时写入 History，避免出现数据库已变化但表格未更新的情况。",
        ],
        [
            """
data.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
data.createdTimestamp =
    QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");

if (!m_patientModel->addPatient(data, &error)) {
    QMessageBox::warning(this, "新增失败", error);
}

m_patientModel->refresh();
logOperation("新增患者：" + data.name + "（" + data.idCard + "）");
            """,
        ],
        [
            (
                "05_patient_editor.png",
                "图 8  新增患者信息表单",
                4.65,
            ),
            (
                "06_validation_error.png",
                "图 9  手机号格式错误的字段校验",
                4.65,
            ),
            (
                "07_after_add.png",
                "图 10  新增成功后患者列表自动刷新",
                6.15,
            ),
        ],
    )

    git_log = """
* c6ee354  2026-11-26  test: add Qt SQL tests and report screenshots
* 0e2da2b  2026-11-26  feat: implement login and patient management UI
* 4d63bb7  2026-11-26  feat: add SQLite schema and patient table model
    """
    current = add_paragraph_after(
        anchors["git"],
        "本次实现按“数据库与模型、界面与操作、测试与报告素材”提交为三次 Git 记录。"
        "当前工作区未配置远程仓库，因此此处如实展示本地提交历史，不将其表述为已经推送 GitHub。",
        left_indent=0.5,
    )
    add_code_after(current, git_log)
    add_paragraph_after(
        current,
        "如后续建立远程仓库，可执行 git remote add origin <仓库地址>，"
        "再执行 git push -u origin master 完成上传。",
        left_indent=0.5,
    )

    current = add_paragraph_after(
        anchors["tests"],
        "程序使用 Qt Test 编写数据库与模型一致性测试，实际结果为 7 项通过、0 项失败、0 项跳过，"
        "总耗时 169 ms。以下为关键测试场景：",
        left_indent=0.5,
    )
    test_lines = [
        ("登录正确/错误凭据：", "正确账号 id 非空；错误密码返回“用户名或密码错误”。通过。"),
        ("必填字段与格式校验：", "空姓名、非法手机号不能保存，界面显示具体原因。通过。"),
        ("重复身份证号：", "第二次插入被 UNIQUE 约束拒绝，数据库中仅保留 1 条记录。通过。"),
        ("查询无结果及恢复：", "查询返回 0 行，清空条件后恢复全部记录。通过。"),
        ("增删改一致性：", "新增后行数 +1；修改后数据库与模型均显示新值；删除后行数恢复且数据库中计数为 0。通过。"),
        ("数据库连接失败：", "连接错误路径时 open 返回 false，errorString 提供可显示的错误信息。通过。"),
    ]
    for index, (label, result) in enumerate(test_lines, 1):
        current = add_runs_after(
            current,
            [(f"{index}) {label}", True), (result, False)],
            size=10.5,
            left_indent=0.7,
        )
    current = add_image_after(
        current,
        "10_history.png",
        "图 11  操作记录与数据写入结果",
        6.15,
    )
    add_code_after(
        current,
        """
PASS : HospitalTests::loginAcceptsAndRejectsCredentials()
PASS : HospitalTests::requiredFieldsAndDuplicateDataAreRejected()
PASS : HospitalTests::queryWithoutResultKeepsModelConsistent()
PASS : HospitalTests::crudKeepsDatabaseAndViewInSync()
PASS : HospitalTests::invalidDatabasePathReportsError()
Totals: 7 passed, 0 failed, 0 skipped
        """,
    )

    add_block(
        anchors["summary"],
        [
            "本次实验将数据库访问、模型和界面分层处理：DatabaseManager 统一管理连接与建表，"
            "PatientTableModel 负责患者数据访问和显示转换，MainWindow 管理页面导航与操作流程，"
            "PatientEditorDialog 负责字段输入和界面校验。这样既保留了 Model/View 的自动同步能力，"
            "也能在同一处处理提交失败、回滚和用户提示。",
            "代表性 AI 辅助过程：问题是新增或修改后只调用 model 编辑接口时，界面可能显示变化但数据库"
            "提交失败。提示词要求“使用 Qt 6 的 QSqlTableModel 实现患者 CRUD，明确处理 submitAll、"
            "错误回滚和视图刷新”。AI 给出的初版框架采用 OnManualSubmit，方向正确，但异常文本过于笼统，"
            "且未完全覆盖重复身份证号和出生日期变化后年龄同步的问题。本人将年龄改为由 DOB 实时计算，"
            "把 ID_CARD 设为唯一约束，在所有提交失败分支调用 revertAll 并返回 SQL 错误，再通过自动测试"
            "同时查询模型行数与数据库真实结果进行验证。",
            "调试中还出现过测试程序使用 QSqlRecord 但未包含对应头文件导致的编译错误，补充头文件后通过；"
            "并确认 SQLite 外键需要每次连接后执行 PRAGMA foreign_keys=ON。最终调试与预期一致：错误输入"
            "不会落库，查询无结果不会破坏模型，连续增删改后数据库和界面保持一致。",
            "目前程序的两个可选模块以医生、科室和操作记录查询的形式实现；后续如继续完善，可增加医生与"
            "科室的完整维护界面、密码修改和分页查询。实验的关键收获是：View 层的刷新不能代替对数据库"
            "提交结果的验证，只有把界面校验、数据库约束、错误处理和自动测试连成闭环，才能保证数据一致性。",
        ],
    )

    appendix_lines = [
        ("附录：项目文件与复现说明", True),
        ("主工程：HospitalManagementSystem/HospitalManagementSystem.pro", False),
        ("测试工程：HospitalManagementSystem/tests/HospitalTests.pro", False),
        ("自动截图工程：HospitalManagementSystem/HospitalCapture.pro", False),
        ("运行方式：使用 Qt Creator 打开主工程，选择 Qt 6 MinGW 套件后构建并运行。", False),
        ("演示账号：admin / admin123；患者数据库由程序首次运行时自动生成。", False),
        ("数据库文件：构建目录下的 hospital.sqlite，首次启动时自动建表并写入演示数据。", False),
        ("数据表：User、Patient、Doctor、Department、History。", False),
        ("登录控制：DatabaseManager::authenticate 使用 QSqlQuery 预处理语句和 SHA-256 密码摘要。", False),
        ("患者列表：PatientTableModel 继承 QSqlTableModel，绑定到 QTableView。", False),
        ("编辑表单：PatientEditorDialog 统一处理新增、修改和字段校验。", False),
        ("数据约束：身份证号唯一，性别、身高、体重和年龄由 CHECK 约束复核。", False),
        ("删除流程：确认后调用 removeRow、submitAll 和 select，取消时不改数据库。", False),
        ("自动测试：HospitalTests.exe 输出 7 passed、0 failed、0 skipped。", False),
        ("编译验证：本机使用 Qt 6.11.2 MinGW 64-bit，源码使用 Qt 6.7.3 已提供的 API。", False),
        ("界面截图：HospitalCapture.exe 在 screenshots 目录生成报告用 PNG。", False),
        ("Git 说明：当前为本地三次提交；配置远程仓库后可推送。", False),
        ("实验环境：Windows 11、Qt Creator、Qt SQL、SQLite。", False),
        ("报告要求：保留原模板文字、表格、图片和页面样式，仅在对应步骤后补充结果。", False),
        ("代码与截图对应关系：截图编号与正文图注一致，项目源码可直接复现。", False),
    ]
    for paragraph, (line, bold) in zip(appendix_paragraphs, appendix_lines):
        paragraph.paragraph_format.left_indent = Inches(0.5)
        paragraph.paragraph_format.line_spacing = 1.5
        run = paragraph.add_run(line)
        run.bold = bold
        run.font.name = "宋体"
        run.font.size = Pt(10.5)
        run.font.color.rgb = RGBColor(0, 0, 0)
        run._element.get_or_add_rPr().rFonts.set(qn("w:eastAsia"), "宋体")

    document.save(OUTPUT)
    print(OUTPUT)


if __name__ == "__main__":
    main()
