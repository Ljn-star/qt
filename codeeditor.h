#ifndef CODEEDITOR_H
#define CODEEDITOR_H

#include <QColor>
#include <QPlainTextEdit>

class QPaintEvent;
class QResizeEvent;

// 带行号栏的文本编辑控件（参考 Qt Code Editor Example），
// 在 calculator.ui 之外通过"提升为(promote)"的方式替换掉 QPlainTextEdit。
class CodeEditor : public QPlainTextEdit
{
    Q_OBJECT

public:
    explicit CodeEditor(QWidget *parent = nullptr);

    int lineNumberAreaWidth() const;
    void lineNumberAreaPaintEvent(QPaintEvent *event);

    void setLineNumberVisible(bool visible);
    bool lineNumberVisible() const { return m_lineNumberVisible; }

    // 修改编辑器背景色，同时刷新行号栏
    void setEditorBackgroundColor(const QColor &color);

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void highlightCurrentLine();
    void updateLineNumberArea(const QRect &rect, int dy);

private:
    QWidget *m_lineNumberArea = nullptr;
    bool m_lineNumberVisible = true;
};

#endif // CODEEDITOR_H
