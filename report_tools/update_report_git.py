from pathlib import Path

from docx import Document
from docx.opc.constants import RELATIONSHIP_TYPE
from docx.oxml import OxmlElement
from docx.oxml.ns import qn
from docx.shared import Pt, RGBColor


REPORT = (
    Path(__file__).resolve().parents[1]
    / "实验3-QtSQL数据库模块与View-Model结构的使用_2026秋_完成版.docx"
)
REPOSITORY_URL = "https://github.com/Ljn-star/qt"


def clear(paragraph) -> None:
    for child in list(paragraph._p):
        if child.tag != qn("w:pPr"):
            paragraph._p.remove(child)


def add_text_run(paragraph, text: str, size: float = 12.0):
    run = paragraph.add_run(text)
    run.font.name = "宋体"
    run.font.size = Pt(size)
    run.font.color.rgb = RGBColor(0, 0, 0)
    run._element.get_or_add_rPr().rFonts.set(qn("w:eastAsia"), "宋体")
    return run


def add_hyperlink(paragraph, url: str, text: str) -> None:
    relationship_id = paragraph.part.relate_to(
        url, RELATIONSHIP_TYPE.HYPERLINK, is_external=True
    )
    hyperlink = OxmlElement("w:hyperlink")
    hyperlink.set(qn("r:id"), relationship_id)

    run = OxmlElement("w:r")
    run_properties = OxmlElement("w:rPr")
    color = OxmlElement("w:color")
    color.set(qn("w:val"), "0563C1")
    underline = OxmlElement("w:u")
    underline.set(qn("w:val"), "single")
    fonts = OxmlElement("w:rFonts")
    fonts.set(qn("w:ascii"), "Consolas")
    fonts.set(qn("w:hAnsi"), "Consolas")
    fonts.set(qn("w:eastAsia"), "等线")
    size = OxmlElement("w:sz")
    size.set(qn("w:val"), "21")
    run_properties.extend([fonts, color, underline, size])
    text_element = OxmlElement("w:t")
    text_element.text = text
    run.extend([run_properties, text_element])
    hyperlink.append(run)
    paragraph._p.append(hyperlink)


def add_code(paragraph, lines: list[str]) -> None:
    for index, line in enumerate(lines):
        if index:
            paragraph.add_run().add_break()
        run = paragraph.add_run(line)
        run.font.name = "Consolas"
        run.font.size = Pt(8.5)
        run.font.color.rgb = RGBColor(44, 62, 80)
        run._element.get_or_add_rPr().rFonts.set(qn("w:eastAsia"), "等线")


def main() -> None:
    document = Document(REPORT)
    cell = document.tables[0].cell(4, 0)

    intro = next(
        p
        for p in cell.paragraphs
        if p.text.startswith("本次实现按") or p.text.startswith("本次实验三源码")
    )
    repository = next(
        p
        for p in cell.paragraphs
        if p.text.startswith("如后续建立") or p.text.startswith("仓库地址")
    )
    log = next(p for p in cell.paragraphs if p.text.lstrip().startswith("*"))

    clear(intro)
    add_text_run(
        intro,
        "本次实验三源码与测试已同步到原有 GitHub 仓库 Ljn-star/qt，"
        "保留实验二文件和既有提交历史。实验三对应的远程提交如下：",
    )

    clear(repository)
    add_text_run(repository, "仓库地址：")
    add_hyperlink(repository, REPOSITORY_URL, REPOSITORY_URL)

    clear(log)
    add_code(
        log,
        [
            "bebafd3  2026-11-26  test: add Qt SQL tests and report screenshots",
            "9fb8e19  2026-11-26  feat: implement login and patient management UI",
            "dc9814c  2026-11-26  feat: add SQLite schema and patient table model",
        ],
    )

    document.save(REPORT)
    print(REPORT)


if __name__ == "__main__":
    main()
