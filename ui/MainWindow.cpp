#include "MainWindow.h"

#include "ui_MainWindow.h"

#include <QProcess>
#include <QFileDialog>
#include <QScrollBar>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), ui(new Ui::MainWindow), m_algo_process(new QProcess(this))
{
	ui->setupUi(this);

	m_algo_process->setWorkingDirectory(QApplication::applicationDirPath());
	m_algo_process->setProgram("cmd.exe");
	m_algo_process->setProcessChannelMode(QProcess::SeparateChannels);

	connect(m_algo_process, &QProcess::readyReadStandardError, [this]() {
		ui->output_log->setStyleSheet("font-weight:bold;color:red");
		ui->output_log->appendPlainText(m_algo_process->readAllStandardError());
		ui->output_log->verticalScrollBar()->setValue(ui->output_log->verticalScrollBar()->maximum()); });
	connect(m_algo_process, &QProcess::readyReadStandardOutput, [this]() {
		ui->output_log->setStyleSheet("color:black");
		ui->output_log->appendPlainText(m_algo_process->readAllStandardOutput());
		ui->output_log->verticalScrollBar()->setValue(ui->output_log->verticalScrollBar()->maximum()); });
	connect(m_algo_process, &QProcess::finished, [this]() {
		ui->output->setPixmap(QPixmap(QApplication::applicationDirPath() + "/output/" + ui->output_file->text()).scaled(ui->output->size(), Qt::KeepAspectRatio));
		ui->depthmap->setPixmap(QPixmap(QApplication::applicationDirPath() + "/output/" + ui->depthmap_file->text()).scaled(ui->depthmap->size(), Qt::KeepAspectRatio));
		ui->_3dview->setPixmap(QPixmap(QApplication::applicationDirPath() + "/output/" + ui->_3dview_file->text()).scaled(ui->_3dview->size(), Qt::KeepAspectRatio));
		ui->focus_stack_btn->setEnabled(true);
		});

	connect(ui->actionFiles, &QAction::triggered, [this]() {
		if (auto files = QFileDialog::getOpenFileNames(this, tr("Select Image Files"), QApplication::applicationDirPath(),
			tr("Images (*.png *.xpm *.jpg)")); !files.isEmpty())
		{
			m_files = files;
			ui->focus_stack_btn->setEnabled(true);
		}});
	connect(ui->focus_stack_btn, &QPushButton::pressed, [this]() { doFocusStack(); });
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::doFocusStack()
{
	ui->focus_stack_btn->setEnabled(false);

	//qDebug() << m_files;
	auto output_folder = QApplication::applicationDirPath() + "/output/";
	if (auto d = QDir(output_folder); !d.exists()) d.mkpath(".");

	QString cmd = "focus-stack.exe --reference=%1 --consistency=%2 --denoise=%3 "
				  "--depthmap-threshold=%4 --depthmap-smooth-xy=%5 --depthmap-smooth-z=%6 --remove-bg=%7 --halo-radius=%8 --3dviewpoint=%9:%10:%11:%12 "
				  "--threads=%13 --batchsize=%14 --wait-images=%15 --output=%16 --depthmap=%17 --3dview=%18 --jpgquality=%19";
	cmd = cmd.arg(QString::number(ui->reference->value()), QString::number(ui->consistency->value()), QString::number(ui->denoise->value()),
		QString::number(ui->depthmap_threshold->value()), QString::number(ui->depthmap_smooth_xy->value()), QString::number(ui->depthmap_smooth_z->value()),
		QString::number(ui->remove_bg->value()), QString::number(ui->halo_ratius->value()), QString::number(ui->_3dviewpoint_x->value()),
		QString::number(ui->_3dviewpoint_y->value()), QString::number(ui->_3dviewpoint_z->value()), QString::number(ui->_3dviewpoint_zscale->value()),
		QString::number(ui->threads->value()), QString::number(ui->batch_size->value()), QString::number(ui->wait_images->value()), output_folder + ui->output_file->text(),
		output_folder + ui->depthmap_file->text(), output_folder + ui->_3dview_file->text(), QString::number(ui->jpgquality->value()));

	if (ui->global_align->isChecked()) cmd += " --global-align";
	if (ui->full_resolution_align->isChecked()) cmd += " --full-resolution-align";
	if (ui->no_whitebalance->isChecked()) cmd += " --no-whitebalance";
	if (ui->no_contrast->isChecked()) cmd += " --no-contrast";
	if (ui->align_only->isChecked()) cmd += " --align-only";
	if (ui->align_keep_size->isChecked()) cmd += " --align-keep-size";

	if (ui->no_opencl->isChecked()) cmd += " --no-opencl";

	if (ui->verbose->isChecked()) cmd += " --verbose";
	if (ui->version->isChecked()) cmd += " --version";
	if (ui->opencv_version->isChecked()) cmd += " --opencv-version";

	if (ui->save_steps->isChecked()) cmd += " --save-steps";
	if (ui->no_crop->isChecked()) cmd += " --nocrop";

	//qDebug() << cmd;

	QStringList args{{"/C"}};
	args.append(cmd);
	args.append(m_files.join(" "));

	//qDebug() << args;

	m_algo_process->setArguments(args);
	m_algo_process->start();
}
