#include "toolbox_coordinates.h"
#include "ui_decimaldegree_to_utm.h"

#include <QDialogButtonBox>
#include <QEvent>
#include <QMimeData>
#include <QDragEnterEvent>
#include <array>
#include <math.h>
#include "decimaldegree_to_utm.h"

ToolboxCoordinates::ToolboxCoordinates()
{
  _widget = new QWidget(nullptr);
  ui = new Ui::decimaldegree_to_utm;

  ui->setupUi(_widget);

  ui->lineEditLat->installEventFilter(this);
  ui->lineEditLon->installEventFilter(this);

  connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &ToolboxCoordinates::onClosed);

  connect(ui->pushButtonSave, &QPushButton::clicked, this,
          &ToolboxCoordinates::on_pushButtonSave_clicked);
}

ToolboxCoordinates::~ToolboxCoordinates()
{
}

void ToolboxCoordinates::init(PJ::PlotDataMapRef& src_data,
                             PJ::TransformsMap& transform_map)
{
  _plot_data = &src_data;
  _transforms = &transform_map;

  _plot_widget = new PJ::PlotWidgetBase(ui->frame);

  auto preview_layout = new QHBoxLayout(ui->framePlotPreview);
  preview_layout->setMargin(6);
  preview_layout->addWidget(_plot_widget);
}

std::pair<QWidget*, PJ::ToolboxPlugin::WidgetType>
ToolboxCoordinates::providedWidget() const
{
  return { _widget, PJ::ToolboxPlugin::FIXED };
}

bool ToolboxCoordinates::onShowWidget()
{
  return true;
}

bool ToolboxCoordinates::eventFilter(QObject* obj, QEvent* ev)
{
  if (ev->type() == QEvent::DragEnter)
  {
    auto event = static_cast<QDragEnterEvent*>(ev);
    const QMimeData* mimeData = event->mimeData();
    QStringList mimeFormats = mimeData->formats();

    for (const QString& format : mimeFormats)
    {
      QByteArray encoded = mimeData->data(format);
      QDataStream stream(&encoded, QIODevice::ReadOnly);

      if (format != "curveslist/add_curve")
      {
        return false;
      }

      QStringList curves;
      while (!stream.atEnd())
      {
        QString curve_name;
        stream >> curve_name;
        if (!curve_name.isEmpty())
        {
          curves.push_back(curve_name);
        }
      }
      if (curves.size() != 1)
      {
        return false;
      }

      _dragging_curve = curves.front();

      if (obj == ui->lineEditLat || obj == ui->lineEditLon)
      {
        event->acceptProposedAction();
        return true;
      }
    }
  }
  else if (ev->type() == QEvent::Drop)
  {
    auto lineEdit = qobject_cast<QLineEdit*>(obj);

    if (!lineEdit)
    {
      return false;
    }
    lineEdit->setText(_dragging_curve);
    if ((obj == ui->lineEditLat && (_dragging_curve.endsWith("latitude") || _dragging_curve.endsWith("lat")) ||
        (obj == ui->lineEditLon && (_dragging_curve.endsWith("longitude") || _dragging_curve.endsWith("lon")))))
    {
      autoFill(_dragging_curve.left(_dragging_curve.lastIndexOf(QChar('/')) + 1));
    }
  }

  return false;
}

void ToolboxCoordinates::autoFill(QString prefix)
{
  QStringList suffix = {"latitude", "longitude", "lat", "lon"};
  std::array<QLineEdit*, 2> lineEdits = { ui->lineEditLat, ui->lineEditLon};
  QStringList names;
  for (int i = 0; i < 4; i++)
  {
    QString name = prefix + suffix[i];
    auto it = _plot_data->numeric.find(name.toStdString());
    if (it != _plot_data->numeric.end())
    {
      names.push_back(name);
    }
  }

  if (names.size() == 2)
  {
    for (int i = 0; i < 2; i++)
    {
      lineEdits[i]->setText(names[i]);
    }
    ui->lineEditOut->setText(prefix);
    ui->pushButtonSave->setEnabled(true);

    generateUTM(PREVIEW);
  }
}

bool ToolboxCoordinates::generateUTM(GenerateType type)
{
  using namespace PJ;

  double unit_scale = 1.0;
  auto transform = std::make_shared<DecimalDegreeToUTM>();

  std::vector<const PlotData*> src_data;
  {
    for (QLineEdit* line : { ui->lineEditLat, ui->lineEditLon})
    {
      auto it = _plot_data->numeric.find(line->text().toStdString());
      if (it == _plot_data->numeric.end())
      {
        return false;
      }
      src_data.push_back(&it->second);
    }
  }

  std::string prefix = ui->lineEditOut->text().toStdString();

  // remove previous cruves bvefore creating new one
  _plot_widget->removeAllCurves();

  _preview_data_utmx.reset(new PlotData(prefix + "utm_x", {}));
  _preview_data_utmy.reset(new PlotData(prefix + "utm_y", {}));

  std::vector<PlotData*> dst_vector = { _preview_data_utmx.get(),
                                        _preview_data_utmy.get()};
  if (type == SAVE)
  {
    dst_vector[0] = &_plot_data->getOrCreateNumeric(prefix + "utm_x", {});
    dst_vector[1] = &_plot_data->getOrCreateNumeric(prefix + "utm_y", {});
  }

  transform->setData(_plot_data, src_data, dst_vector);
  transform->calculate();
  transform->setScale(unit_scale);


  if (type == PREVIEW)
  {
    _plot_widget->removeAllCurves();
    for (auto dst_data : dst_vector)
    {
      _plot_widget->addCurve(dst_data->plotName(), *dst_data);
    }
    _plot_widget->resetZoom();
  }

  if (type == SAVE)
  {
    _transforms->insert({ prefix + "UTM", transform });

    emit plotCreated(prefix + "utm_x");
    emit plotCreated(prefix + "utm_y");
  }
  return true;
}

void ToolboxCoordinates::on_pushButtonSave_clicked()
{
  generateUTM(SAVE);

  ui->lineEditLat->setText({});
  ui->lineEditLon->setText({});
  ui->lineEditOut->setText({});
  _plot_widget->removeAllCurves();

  emit this->closed();
}

void ToolboxCoordinates::onParametersChanged()
{
  if (ui->lineEditLat->text().isEmpty() || ui->lineEditLon->text().isEmpty() ||
      ui->lineEditOut->text().isEmpty())
  {
    ui->pushButtonSave->setEnabled(false);
    return;
  }
  bool valid = generateUTM(PREVIEW);
  ui->pushButtonSave->setEnabled(valid);
}

void ToolboxCoordinates::onClosed()
{
  emit this->closed();
}
