#include "FunctionBlockDiagramWidget.h"

#include <QDebug>
#include <QVBoxLayout>

FunctionBlockDiagramWidget::FunctionBlockDiagramWidget( QWidget * parent )
    : QWidget( parent )
{
    auto * vBox = new QVBoxLayout( this );
    m_graphicsView = new QGraphicsView{};
    vBox->addWidget( m_graphicsView );

    QSize sceneSize { 1000, 800 };
    m_scene = new QGraphicsScene{};
    m_graphicsView->setScene( m_scene );
    m_scene->setSceneRect( 0, 0, sceneSize.width(), sceneSize.height() );

    QBrush brush( Qt::black, Qt::SolidPattern );

    /** TODO: установить точечный фон */
//    QPixmap texture( 11, 11 );
//    texture.fill( Qt::black );
//    QImage image = texture.toImage();
//    image.setPixel( texture.size().width() / 2, texture.size().height() / 2, Qt::white );
//    texture.fromImage( image );

//    brush.setTexture( texture );
    m_graphicsView->setBackgroundBrush( brush );

    m_graphicsView->setRenderHint( QPainter::Antialiasing );
    m_graphicsView->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    m_graphicsView->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    m_graphicsView->setCacheMode( QGraphicsView::CacheBackground ); // Кэш фона
    m_graphicsView->setViewportUpdateMode( QGraphicsView::BoundingRectViewportUpdate );

    m_functionGraph = new FunctionGraph{ this };
    connect( m_functionGraph, &FunctionGraph::updated,
             this, &FunctionBlockDiagramWidget::graphUpdated,
             Qt::DirectConnection );
    connect( m_functionGraph, &FunctionGraph::connectionChanged,
             this, &FunctionBlockDiagramWidget::setConnection,
             Qt::DirectConnection );

    m_functionGraph->loadVertices(
        {
            { "Temperature", 3, 1 },
            { "Motor", 2, 2 },
            { "Pressure", 1, 1 },
            { "Kek", 2, 2 },
            { "Lol", 0, 0 },
        } );

}

void FunctionBlockDiagramWidget::graphUpdated()
{
    auto vertices = m_functionGraph->getVertices();

    QPointF position = m_blockMap.empty()
            ? m_blockAppearPoint
            : m_blockMap.back()->pos() + QPointF( 0, m_blockMap.back()->size().height() )
              + m_stepAppearPoint;
    for ( int i = m_blockMap.size(); i < vertices.size(); i++ )
    {
        auto & vertex = vertices[i];
        auto * blockItem = new FunctionBlockItem( vertex.inputPins.size(),
                                                  vertex.outputPins.size(),
                                                  this );
        position += QPointF( 0, blockItem->size().height() / 2 );
        {
            m_blockMap.push_back( blockItem );
            m_scene->addItem( blockItem );
            blockItem->setPos( position );
            connect( blockItem, & FunctionBlockItem::pinClicked,
                     this,
                     [ this, i ] ( bool isIn, int pinIndex ) -> void
            {
                blockPinClicked( isIn, i, pinIndex );
            }, Qt::DirectConnection );
        }
        position += QPointF( 0, blockItem->size().height() / 2 )
                + m_stepAppearPoint;
    }

//    for ( int i = 0; i < vertices.size(); i++ )
//    {
//        auto & vertex = vertices[ i ];
//        for ( int j = 0; j < vertex.inputPins.size(); j++ )
//        {
//            auto & inPin = vertex.inputPins[ j ];
//            if ( inPin.has_value() )
//            {
//                setConnection( inPin.value(),
//                               SFunctionPinIndex{ i, j }, true );
//            }
//        }
//    }
}

void FunctionBlockDiagramWidget::blockPinClicked( bool isIn,
                                                  int blockIndex,
                                                  int pinIndex )
{
    auto & pinSelected = ( isIn ) ? m_inPinSelected : m_outPinSelected;
    if ( pinSelected.has_value() )
    {
        setPinSelected( isIn, pinSelected.value(), false );
    }
    pinSelected = SFunctionPinIndex{ blockIndex, pinIndex };
    setPinSelected( isIn, pinSelected.value(), true );

    if ( m_inPinSelected.has_value() && m_outPinSelected.has_value() )
    {
        setPinSelected( true, m_inPinSelected.value(), false );
        setPinSelected( false, m_outPinSelected.value(), false );

        m_functionGraph->connectVertices( m_inPinSelected.value(),
                                          m_outPinSelected.value() );

        m_inPinSelected.reset();
        m_outPinSelected.reset();
    }
}

void FunctionBlockDiagramWidget::setPinSelected(
        bool isIn,
        const SFunctionPinIndex & pairIndex,
        bool isSelected )
{
    m_blockMap[ pairIndex.func ]->setPinSelected( isIn, pairIndex.pin, isSelected );
}

void FunctionBlockDiagramWidget::setConnection( const SFunctionPinIndex & inFuncIndex,
                                                const SFunctionPinIndex & outFuncIndex,
                                                bool hasConnection )
{
    SConnection keyCon{ inFuncIndex, outFuncIndex };
    if ( hasConnection )
    {
        if ( ! m_linesMap.contains( keyCon ) )
        {
            auto * lineItem = new ConnectionItem( this );
            m_linesMap.insert( keyCon, lineItem );
            m_scene->addItem( lineItem );
            auto * inBlockItem = m_blockMap[ inFuncIndex.func ];
            auto * outBlockItem = m_blockMap[ outFuncIndex.func ];
            lineItem->setLine(
                        QLineF(
                            inBlockItem->getEdgePinPoint( true, inFuncIndex.pin ),
                            outBlockItem->getEdgePinPoint( false, outFuncIndex.pin )
                            )
                        );
        }
    }
}
