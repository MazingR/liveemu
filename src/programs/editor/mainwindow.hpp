#pragma once

#include <QMainWindow>

class QAction;
class QMenu;
class QPlainTextEdit;
class QSessionManager;

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow();

	void loadFile(const QString &fileName);

protected:
	void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;

private slots:
	void newFile();
	void open();
	bool save();
	bool saveAs();
	void about();
	void documentWasModified();
#ifndef QT_NO_SESSIONMANAGER
	void commitData(QSessionManager &);
#endif

private:
	void createActions();
	void createStatusBar();
	void readSettings();
	void writeSettings();
	bool maybeSave();
	bool saveFile(const QString &fileName);
	void setCurrentFile(const QString &fileName);
	QString strippedName(const QString &fullFileName);

	QPlainTextEdit *textEdit;
	QString curFile;
};