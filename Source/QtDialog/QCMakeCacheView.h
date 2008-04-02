/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile: QCMakeCacheView.h,v $
  Language:  C++
<<<<<<< QCMakeCacheView.h
  Date:      $Date: 2008/02/15 00:58:31 $
  Version:   $Revision: 1.17 $
=======
  Date:      $Date: 2008-04-02 19:28:17 $
  Version:   $Revision: 1.18 $
>>>>>>> 1.18

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef QCMakeCacheView_h
#define QCMakeCacheView_h

#include "QCMake.h"
#include <QTableView>
#include <QAbstractTableModel>
#include <QCheckBox>
#include <QLineEdit>
#include <QItemDelegate>
#include <QSortFilterProxyModel>
#include <QCompleter>

class QCMakeCacheModel;
class QToolButton;


/// Qt view class for cache properties
class QCMakeCacheView : public QTableView
{
  Q_OBJECT
public:
  QCMakeCacheView(QWidget* p);

  QCMakeCacheModel* cacheModel() const;
  bool showAdvanced() const;

public slots:
  void setShowAdvanced(bool);
  void setSearchFilter(const QString&);

protected:
  QModelIndex moveCursor(CursorAction, Qt::KeyboardModifiers);
  void showEvent(QShowEvent* e);
  bool Init;
  QCMakeCacheModel* CacheModel;
  QSortFilterProxyModel* AdvancedFilter;
  QSortFilterProxyModel* SearchFilter;
};

/// Qt model class for cache properties
class QCMakeCacheModel : public QAbstractTableModel
{
  Q_OBJECT
public:
  QCMakeCacheModel(QObject* parent);
  ~QCMakeCacheModel();

  enum { HelpRole = Qt::UserRole, TypeRole, AdvancedRole };

public slots:
  void setProperties(const QCMakeCachePropertyList& props);
  void clear();
  void setEditEnabled(bool);
  bool removeRows(int row, int count, const QModelIndex& idx = QModelIndex());
  bool insertRows(int row, int num, const QModelIndex&);

public:
  // satisfy [pure] virtuals
  int columnCount (const QModelIndex& parent) const;
  QVariant data (const QModelIndex& index, int role = Qt::DisplayRole) const;
  QModelIndex parent (const QModelIndex& index) const;
  int rowCount (const QModelIndex& parent = QModelIndex()) const;
  QVariant headerData (int section, Qt::Orientation orient, int role) const;
  Qt::ItemFlags flags (const QModelIndex& index) const;
  bool setData (const QModelIndex& index, const QVariant& value, int role);
  QModelIndex buddy (const QModelIndex& index) const;

  // get the properties
  QCMakeCachePropertyList properties() const;
  
  // editing enabled
  bool editEnabled() const;

  int newCount() const;

protected:
  QCMakeCachePropertyList Properties;
  int NewCount;
  bool EditEnabled;
};

/// Qt delegate class for interaction (or other customization) 
/// with cache properties
class QCMakeCacheModelDelegate : public QItemDelegate
{
  Q_OBJECT
public:
  QCMakeCacheModelDelegate(QObject* p);
  /// create our own editors for cache properties
  QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, 
      const QModelIndex& index ) const;
  bool editorEvent (QEvent* event, QAbstractItemModel* model, 
       const QStyleOptionViewItem& option, const QModelIndex& index);
  bool eventFilter(QObject* object, QEvent* event);
protected slots:
  void setFileDialogFlag(bool);
protected:
  bool FileDialogFlag;
};

/// Editor widget for editing paths or file paths
class QCMakeCacheFileEditor : public QLineEdit
{
  Q_OBJECT
public:
  QCMakeCacheFileEditor(QWidget* p, const QString& var);
protected slots:
  virtual void chooseFile() = 0;
signals:
  void fileDialogExists(bool);
protected:
  void resizeEvent(QResizeEvent* e);
  QToolButton* ToolButton;
  QString Variable;
};

class QCMakeCachePathEditor : public QCMakeCacheFileEditor
{
  Q_OBJECT
public:
  QCMakeCachePathEditor(QWidget* p = NULL, const QString& var = QString());
  void chooseFile();
};

class QCMakeCacheFilePathEditor : public QCMakeCacheFileEditor
{
  Q_OBJECT
public:
  QCMakeCacheFilePathEditor(QWidget* p = NULL, const QString& var = QString());
  void chooseFile();
};

/// completer class that returns native cmake paths
class QCMakeFileCompleter : public QCompleter
{
public:
  QCMakeFileCompleter(QObject* o, bool dirs);
  virtual QString pathFromIndex(const QModelIndex& idx) const;
};

#endif

