#pragma once

#include <QtPlugin>
#include <thread>
#include "PlotJuggler/toolbox_base.h"
#include "PlotJuggler/plotwidget_base.h"
#include "decimaldegree_to_utm.h"

namespace Ui
{
class decimaldegree_to_utm;
}

class ToolboxCoordinates : public PJ::ToolboxPlugin
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "kulmerdominik.PlotJuggler3.Toolbox")
  Q_INTERFACES(PJ::ToolboxPlugin)

public:
  ToolboxCoordinates();

  ~ToolboxCoordinates() override;

  const char* name() const override
  {
    return "Deciaml Degree to UTM";
  }

  void init(PJ::PlotDataMapRef& src_data, PJ::TransformsMap& transform_map) override;

  std::pair<QWidget*, WidgetType> providedWidget() const override;

public slots:

  bool onShowWidget() override;

private slots:

  void on_pushButtonSave_clicked();

  void onParametersChanged();

  void onClosed();

private:
  QWidget* _widget;
  Ui::decimaldegree_to_utm* ui;

  bool eventFilter(QObject* obj, QEvent* event) override;

  QString _dragging_curve;

  void autoFill(QString prefix);

  PJ::PlotWidgetBase* _plot_widget = nullptr;

  PJ::PlotDataMapRef* _plot_data = nullptr;

  PJ::TransformsMap* _transforms = nullptr;

  std::unique_ptr<PlotData> _preview_data_utmx;
  std::unique_ptr<PlotData> _preview_data_utmy;

  enum GenerateType
  {
    PREVIEW,
    SAVE
  };

  bool generateUTM(GenerateType type);
};
