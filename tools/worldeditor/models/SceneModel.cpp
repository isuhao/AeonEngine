/*
Copyright (C) 2015,2018 Rodrigo Jose Hernandez Cordoba

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include <QIcon>
#include <QFile>
#include <QMimeData>
#include <QDataStream>
#include <QByteArray>
#include <QXmlStreamWriter>
#include <QTextStream>
#include "SceneModel.h"
#include "aeongames/Scene.h"

namespace AeonGames
{
    SceneModel::SceneModel ( QObject *parent ) :
        QAbstractItemModel ( parent )
    {
    }

    SceneModel::~SceneModel() = default;

    QVariant SceneModel::headerData ( int section, Qt::Orientation orientation, int role ) const
    {
        if ( ( orientation == Qt::Horizontal ) && ( role == Qt::DisplayRole ) )
        {
            switch ( section )
            {
            case 0:
                return QString ( "Node" );
            default:
                return QVariant();
            }
        }
        return QVariant();
    }

    QModelIndex SceneModel::index ( int row, int column, const QModelIndex & parent ) const
    {
        if ( !parent.isValid() )
        {
            if ( row < static_cast<int> ( mScene.GetChildrenCount() ) )
            {
                return createIndex ( row, column, &const_cast<SceneModel*> ( this )->mScene[row] );
            }
        }
        else
        {
            Scene::Node* node = reinterpret_cast<Scene::Node*> ( parent.internalPointer() );
            if ( row < static_cast<int> ( node->GetChildrenCount() ) )
            {
                return createIndex ( row, column, & ( node->GetChild ( row ) ) );
            }
        }
        return QModelIndex();
    }

    QModelIndex SceneModel::parent ( const QModelIndex & index ) const
    {
        if ( index.isValid() )
        {
            Scene::Node* node = reinterpret_cast<Scene::Node*> ( index.internalPointer() );
            Scene::Node* node_parent = node->GetParent();
            if ( node_parent != nullptr )
            {
                return createIndex ( static_cast<int> ( node->GetIndex() ), 0, node_parent );
            }
        }
        return QModelIndex();
    }

    int SceneModel::rowCount ( const QModelIndex & index ) const
    {
        if ( index.isValid() )
        {
            return static_cast<int> ( reinterpret_cast<Scene::Node*> ( index.internalPointer() )->GetChildrenCount() );
        }
        return static_cast<int> ( mScene.GetChildrenCount() );
    }

    int SceneModel::columnCount ( const QModelIndex & index ) const
    {
        return 1;
    }

    bool SceneModel::hasChildren ( const QModelIndex & index ) const
    {
        if ( index.isValid() )
        {
            return ( reinterpret_cast<Scene::Node*> ( index.internalPointer() )->GetChildrenCount() > 0 );
        }
        return ( mScene.GetChildrenCount() > 0 );
    }

    QVariant SceneModel::data ( const QModelIndex & index, int role ) const
    {
        if ( ( role == Qt::DisplayRole ) || ( role == Qt::EditRole ) )
        {
            if ( index.isValid() )
            {
                switch ( index.column() )
                {
                case 0:
                    return QString ( reinterpret_cast<Scene::Node*> ( index.internalPointer() )->GetName().c_str() );
                    break;
                }
            }
        }
        else if ( role == Qt::DecorationRole )
        {
            return QIcon ( ":/icons/icon_node" );
        }
        else if ( role == Qt::UserRole )
        {
            return qVariantFromValue ( index.internalPointer() );
        }
        return QVariant();
    }

    Qt::ItemFlags SceneModel::flags ( const QModelIndex & index ) const
    {
        if ( index.isValid() )
        {
            return QAbstractItemModel::flags ( index ) | Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
        }
        return QAbstractItemModel::flags ( index ) | Qt::ItemIsDropEnabled;
    }

    bool SceneModel::setData ( const QModelIndex & index, const QVariant & value, int role )
    {
        Scene::Node* node = reinterpret_cast<Scene::Node*> ( index.internalPointer() );
        if  ( role == Qt::EditRole )
        {
            node->SetName ( value.toString().toUtf8().constData() );
            emit dataChanged ( index, index );
            return true;
        }
        return false;
    }

    bool SceneModel::moveRows ( const QModelIndex & sourceParent, int sourceRow, int count, const QModelIndex & destinationParent, int destinationRow )
    {
        if ( sourceParent.isValid() && destinationParent.isValid() )
        {
            // Moving between nodes
            Scene::Node* source = reinterpret_cast<Scene::Node*> ( sourceParent.internalPointer() );
            Scene::Node* destination = reinterpret_cast<Scene::Node*> ( destinationParent.internalPointer() );
            if ( beginMoveRows ( sourceParent, sourceRow, ( sourceRow + count ) - 1, destinationParent, destinationRow ) )
            {
                for ( int i = 0; i < count; ++i )
                {
                    auto node = source->GetChild ( sourceRow );
                    destination->Move ( destinationRow + i, std::move ( *source ) );
                }
                endMoveRows();
            }
            else
            {
                return false;
            }
        }
        else if ( sourceParent.isValid() )
        {
            // Moving from a node to the scene
            Scene::Node* source = reinterpret_cast<Scene::Node*> ( sourceParent.internalPointer() );
            if ( beginMoveRows ( sourceParent, sourceRow, ( sourceRow + count ) - 1, destinationParent, destinationRow ) )
            {
                for ( int i = 0; i < count; ++i )
                {
                    auto node = source->GetChild ( sourceRow );
                    mScene.Move ( destinationRow, std::move ( *source ) );
                }
                endMoveRows();
            }
            else
            {
                return false;
            }
        }
        else if ( destinationParent.isValid() )
        {
            // Moving from the scene to a node
            Scene::Node* destination = reinterpret_cast<Scene::Node*> ( destinationParent.internalPointer() );
            if ( beginMoveRows ( sourceParent, sourceRow, ( sourceRow + count ) - 1, destinationParent, destinationRow ) )
            {
                for ( int i = 0; i < count; ++i )
                {
                    Scene::Node* node = &mScene.GetChild ( sourceRow );
                    destination->Move ( destinationRow, std::move ( *node ) );
                }
                endMoveRows();
            }
            else
            {
                return false;
            }
        }
        else
        {
            /* This is the case when a row is moved up or down directly at the scene*/
            if ( beginMoveRows ( sourceParent, sourceRow, ( sourceRow + count ) - 1, destinationParent, destinationRow ) )
            {
                for ( int i = 0; i < count; ++i )
                {
                    Scene::Node* node = &mScene.GetChild ( sourceRow );
                    mScene.Move ( destinationRow, std::move ( *node ) );
                }
                endMoveRows();
            }
            else
            {
                return false;
            }
        }
        return true;
    }

    Qt::DropActions SceneModel::supportedDropActions() const
    {
        return Qt::MoveAction;
    }

    bool SceneModel::dropMimeData ( const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent )
    {
        QString format ( "application/x-aeon-editor-node" );
        // check if the action is supported
        if ( !data || ( action != Qt::MoveAction ) )
        {
            return false;
        }
        // check if the format is supported
        if ( !data->hasFormat ( format ) )
        {
            return false;
        }
        if ( ( row > rowCount ( parent ) ) || ( row < 0 ) )
        {
            row = rowCount ( parent );
        }
        QByteArray byteArray = data->data ( format );
        QDataStream dataStream ( &byteArray, QIODevice::ReadOnly );
        int count;
        dataStream >> count;
        for ( int i = 0; i < count; ++i )
        {
            Scene::Node* pointer;
            dataStream.readRawData ( reinterpret_cast<char*> ( &pointer ), sizeof ( void* ) );
            QModelIndex model_index = createIndex ( static_cast<int> ( pointer->GetIndex() ), 0, pointer );
            moveRow ( this->parent ( model_index ), static_cast<int> ( pointer->GetIndex() ), parent, row );
        }
        return true;
    }

    QMimeData * SceneModel::mimeData ( const QModelIndexList & indexes ) const
    {
        QMimeData* data = nullptr;
        if ( !indexes.isEmpty() )
        {
            QByteArray byteArray;
            byteArray.reserve ( ( indexes.count() + 1 ) *sizeof ( void* ) );
            QDataStream dataStream ( &byteArray, QIODevice::WriteOnly );
            dataStream << indexes.count();
            for ( auto i = indexes.cbegin(); i != indexes.cend(); ++i )
            {
                void* pointer = ( *i ).internalPointer();
                dataStream.writeRawData ( reinterpret_cast<char*> ( &pointer ), sizeof ( void* ) );
            }
            QStringList types = mimeTypes();
            data = new QMimeData();
            data->setData ( types[0], byteArray );
        }
        return data;
    }

    QStringList SceneModel::mimeTypes() const
    {
        QStringList types;
        types << "application/x-aeon-editor-node";
        return types;
    }

    void SceneModel::InsertNode ( int row, const QModelIndex & parent )
    {
        beginInsertRows ( parent, row, row );
        if ( parent.isValid() )
        {
            reinterpret_cast<Scene::Node*> ( parent.internalPointer() )->Insert ( row, {} );
        }
        else
        {
            mScene.Insert ( row, {} );
        }
        endInsertRows();
    }

    void SceneModel::RemoveNode ( int row, const QModelIndex & parent )
    {
        beginRemoveRows ( parent, row, row );
        if ( parent.isValid() )
        {
            reinterpret_cast<Scene::Node*> ( parent.internalPointer() )->Erase ( row );
        }
        else
        {
            mScene.Erase ( row );
        }
        endRemoveRows();
    }

    const Scene& SceneModel::GetScene() const
    {
        return mScene;
    }
}
