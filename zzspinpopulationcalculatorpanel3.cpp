#include "zzspinpopulationcalculatorpanel3.h"
#include "graphmodelgraphicsitem4.h"
#include "graphmodelgraphicsitem3.h"
#include "zoomableview.h"
#include "graphmodel.h"
#include "bonditem.h"

#include <QCoreApplication>
#include <QVBoxLayout>
#include <QPushButton>
#include <QFontMetricsF>
#include "utils.h"
#include "widget.h"
#include <QPainter>
#include <iostream>
#include <fstream>
#include <QFile>
using namespace std;
#include <ostream>
#include <time.h>




#include "symmetryoperators.h"

#include "zzpolynomialcalculator.h"
#include "gzip_helper.h"

#include <QProgressDialog>
#include <QMessageBox>
#include <QtSvg>

inline uint qHash(const QPoint &key)
{
    QString str = QString("(%1,%2)").arg(key.x()).arg(key.y());
    QString hkey = QString(QCryptographicHash::hash(str.toStdString().c_str(),QCryptographicHash::Md5).toHex());
    return qHash(hkey);
}

ZZSpinpopulaitonCalculatorPanel3::ZZSpinpopulaitonCalculatorPanel3(QWidget *parent) : QWidget(parent)

{
    QVBoxLayout *layout = new QVBoxLayout();
    layout->setMargin(0);
    scene = new QGraphicsScene(this);


    ZoomableView* viewer = new ZoomableView(scene);
    viewer->setZoomable(true);
    viewer->zoom(1.5);
    view = viewer;
    view->setMouseTracking(true);
    layout->addWidget(view);


    m_calc3 = new QPushButton(tr("List prescentage"));
    m_calc4 = new QPushButton(tr("Spinpopulation"));



    connect(m_calc3, SIGNAL(clicked()), this, SLOT(list()));
    connect(m_calc4, SIGNAL(clicked()), this, SLOT(calc3()));



    QHBoxLayout *layout_h = new QHBoxLayout();


    layout_h->addStretch(1);


    layout_h->addWidget(m_calc4);
    layout_h->addWidget(m_calc3);


    layout_h->addStretch(1);

    layout->addLayout(layout_h);
    layout->addSpacing(10);
    this->setLayout(layout);




}

ZZSpinpopulaitonCalculatorPanel3::~ZZSpinpopulaitonCalculatorPanel3()
{

}



void ZZSpinpopulaitonCalculatorPanel3::setGraph(GraphModel *graph)
{
    clear_all_things();


    m_graph = graph;
    graph_item = new GraphModelGraphicsItem3(m_graph);




//    graph_item->setAtomNumberVisible(true);
    graph_item->setBondSelectable(false);
    graph_item->setTagVisible(false);
//    if (m_rotateGrid)
//    {
//        graph_item->setGridRotated(m_rotateGrid);
//    }

    scene->addItem(graph_item);
    scene->update();
    view->centerOn(graph_item);
    view->update();
    scene->setSceneRect(scene->itemsBoundingRect());

}


void ZZSpinpopulaitonCalculatorPanel3::exportSVG(QString filename, bool compressed)
{
    QSvgGenerator svgGen;

    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    svgGen.setOutputDevice(&buffer);


    QRectF rect = scene->itemsBoundingRect();
    QGraphicsRectItem *bg=new QGraphicsRectItem(rect.x()-50, rect.y()-50, rect.width()+100, rect.height()+100,0);
    bg->setPen(QPen(Qt::NoPen));
    bg->setBrush(QBrush(QColor("white")));
    bg->setZValue(-50);


;


    scene->addItem(bg);

    QRectF rr = bg->rect();
    svgGen.setTitle("ZZ Aromaticity Export");
//    svgGen.setResolution(300);
    svgGen.setSize(QSize(rr.width(),rr.height()));
    svgGen.setViewBox(rr);

    QPainter painter;
    painter.begin(&svgGen);




    scene->render(&painter,rr,rr);

    painter.end();

    scene->removeItem(bg);
    delete bg;

    QFile file( filename );
    file.open( QIODevice::WriteOnly );
    if (compressed)
    {
        QByteArray compressed = compress(byteArray);
        file.write( compressed );
    }
    else
    {
        file.write( byteArray );
    }
    file.close();

}





void ZZSpinpopulaitonCalculatorPanel3::mouseDoubleClickEvent(QMouseEvent *event)
{
    ZZSpinpopulaitonCalculatorPanel3::mouseDoubleClickEvent(event);
}


void ZZSpinpopulaitonCalculatorPanel3::mouseMoveEvent(QMouseEvent *event)
{
    ZZSpinpopulaitonCalculatorPanel3::mouseMoveEvent(event);
}
void ZZSpinpopulaitonCalculatorPanel3::mousePressEvent(QMouseEvent *event)
{

  ZZSpinpopulaitonCalculatorPanel3::mousePressEvent(event);
}


void ZZSpinpopulaitonCalculatorPanel3::paintEvent(QPaintEvent *e)
{
   QPainter painter(this);

   painter.setPen(Qt::green);               //指定畫筆顏色

   painter.setPen(QPen(Qt::red, 5));        //指定畫筆顏色和粗細
   painter.setBrush(Qt::red);              //指定筆刷顏色，可以想成物體的填充色
   painter.drawEllipse(100, 300, 20, 20);//指定x、y、width、height的矩形，繪製橢圓於內

 }


void ZZSpinpopulaitonCalculatorPanel3::calc3()
{
    clearText();
    red_atom_coordinate.clear();
    blue_atom_coordinate.clear();
    spin_polynomial_blue.clear();
    spin_polynomial_red.clear();
    m_textItems.clear();
    red_circle.clear();
    QProgressDialog progress("Task in progress...", "Cancel", 0, 2, this);
    progress.setWindowModality(Qt::WindowModal);
    progress.setValue(1);
    progress.show();
    QCoreApplication::processEvents();

    QList<EdgeModel> edges = m_graph ->getAllEdges();
    edgessize = edges.size() ;
    cout << "!!!!!!!!!Original edge size = " << edgessize <<endl ;
    QList<GraphModel*> new_graph ;
    QList<EdgeModel> removed_edges = m_graph->m_hidden_edges;
    QList<int> connect_edges ;

   bool cancelled = false;

  GraphModelGraphicsItem4 *graph_item2 =  new GraphModelGraphicsItem4(m_graph);

  cout << " have " << graph_item2->atom_coordinate.size() << " atoms " << endl;



       //define_red_blue_atom*********************

        for(int j=0 ; j<graph_item2->atom_coordinate.size();j++)
        {

           for(int k=0;k<edges.size();k++)
           {
               if(
                    graph_item2->atom_coordinate[j] == graph_item2->getStartPoint(edges[k])

                    )
               {

                 connect_edges.append(k);

               }

               else if (
                          graph_item2->atom_coordinate[j] == graph_item2->getEndPoint(edges[k])
                        )
               {

                 connect_edges.append(k);

               }

           }

        for(int k=0 ;k<connect_edges.size();k++)
        {
                 edges.removeAt(connect_edges[connect_edges.size()-k-1]);
        }
              GraphModel* gm_child = new GraphModel(edges,removed_edges,m_graph,GraphModel::Single, 0);
              ZZPolynomialCalculator *workerThread = new ZZPolynomialCalculator(gm_child, this);

               workerThread->serialRun();

              if(workerThread->getTotal().toDouble()>0)
                    {
                       red_atom_coordinate.append(graph_item2->atom_coordinate[j]);

                    }
              else
              {
                  blue_atom_coordinate.append(graph_item2->atom_coordinate[j]);
              }
               edges.clear();

               edges =  m_graph ->getAllEdges();

               connect_edges.clear();
          }

        for(int i=0 ;i<red_atom_coordinate.size();i++)
        {
            spin_polynomial_red.append(0.0);

        }
        for(int i=0 ;i<blue_atom_coordinate.size();i++)
        {
            spin_polynomial_blue.append(0.0);
        }


cout << "spin pop red atom size = " << red_atom_coordinate.size()<<endl;

cout << "spin pop blue atom size = " << blue_atom_coordinate.size()<<endl;

    progress.setRange(0,blue_atom_coordinate.size());

    for(int i=0 ; i<blue_atom_coordinate.size();i++)
    {
        QCoreApplication::processEvents();

        progress.setValue(i);

        if (progress.wasCanceled())
            {
              cancelled = true ;
              break;
            }


        for(int j=0 ; j<red_atom_coordinate.size();j++)
         {
          for(int l=0 ; l<j ; l++)
          {

             for(int k=0;k<edges.size();k++)
             {
               if(   (blue_atom_coordinate[i] == graph_item2->getStartPoint(edges[k]) ) ||
                     (red_atom_coordinate[j] == graph_item2->getStartPoint(edges[k]) ) ||
                     (red_atom_coordinate[l] == graph_item2->getStartPoint(edges[k]) )
                     )

               {
                 connect_edges.append(k);
               }

               else if (  (blue_atom_coordinate[i]    == graph_item2->getEndPoint(edges[k])) ||
                          (red_atom_coordinate[j]   == graph_item2->getEndPoint(edges[k])) ||
                          (red_atom_coordinate[l]   == graph_item2->getEndPoint(edges[k]))
                          )

               {
                 connect_edges.append(k);
               }

              }



        for(int k=0 ;k<connect_edges.size();k++)
        {
                 edges.removeAt(connect_edges[connect_edges.size()-k-1]);
        }

              GraphModel* gm_child = new GraphModel(edges,removed_edges,m_graph,GraphModel::Single, 0);
              ZZPolynomialCalculator *workerThread = new ZZPolynomialCalculator(gm_child, this);


//              GraphModelGraphicsItem4     *graph_item2 =  new GraphModelGraphicsItem4(gm_child); // check
//              graph_item2->setPos(120*(2*j+i+2*l)-440,80*(i+j-l));
//              scene->addItem(graph_item2);

               workerThread->serialRun();
              if(workerThread->getTotal().toDouble()>0)
                    {

                      spin_polynomial_blue[i]  += (workerThread->getTotal().toDouble());
                      spin_polynomial_red[j] += (workerThread->getTotal().toDouble());
                      spin_polynomial_red[l] += (workerThread->getTotal().toDouble());
//              cout<<"red polynomial  " <<i<< " = " <<spin_polynomial_red[i]<<endl;
//              cout<<"blue polynomial "<<j <<" = "<<spin_polynomial_blue[j]<<endl;
                    }

               edges.clear();

               edges =  m_graph ->getAllEdges();

               connect_edges.clear();

         }
       }
      }

//    double sum = 0.0 ;

//    for(int k=0 ; k<spin_polynomial_red.size();k++)   // find  red sum
//    {
//      if(spin_polynomial_red[k]>0)
//      {
//       cout << "sp poly red  " << k << " = " << spin_polynomial_red[k]<<endl;
//      }
//       sum += spin_polynomial_red[k];

//    }

//    cout<<"sum= "<< sum << endl ;

//    if(sum == 0.0)
//    {     QPointF standard_p ;
//        standard_p.setX(graph_item2->x());
//        standard_p.setY(graph_item2->y());
//           QString subTotal = QString("This structure = 0 ");
//           QGraphicsTextItem *text = new QGraphicsTextItem(subTotal);
//              text ->setDefaultTextColor("red");
//              text->setPos(standard_p.x(),standard_p.y()-50);
//              scene->addItem(text);
//              m_textItems.append(text);
//              cout<<"sum= "<< sum << endl ;

//    }

//    for(int k=0 ; k<spin_polynomial_red.size();k++)   // divide red sum , get precent
//    {
//       spin_polynomial_red[k]= spin_polynomial_red[k]/sum ;

//    }


//    RedMax = 0.0 ;

//    for(int k=0 ; k<spin_polynomial_red.size();k++)   // find RedMaxinum number in "all" list
//    {
//         if(spin_polynomial_red[k]> RedMax)
//           {
//             RedMax = spin_polynomial_red[k];
//            }

//    }

//    cout << "RedMax = " << RedMax << endl ;
//    RedMin = 100.0;

//    for(int k=0 ; k<spin_polynomial_red.size();k++)   // find minimum number in "all" list
//    {
//          if (spin_polynomial_red[k]< RedMin )
//          {
//             RedMin = spin_polynomial_red[k];
//          }
//    }
//         cout << "RedMin = " << RedMin << endl;
//         for(int k=0 ; k<spin_polynomial_red.size();k++)   //  normalize circle size
//         {
//               spin_polynomial_red[k] =  spin_polynomial_red[k] / RedMax ;
//         }

//        for(int i=0; i<spin_polynomial_red.size() ; i++)
//        {
//          if(spin_polynomial_red[i]>0)
//          {
//          double k=25*spin_polynomial_red[i];
//          QBrush redBrush(Qt::red);
//          QPen redpen(Qt::red);
//          red_circle.append(scene->addEllipse(red_atom_coordinate[i].x()-(k-1)/2.0,red_atom_coordinate[i].y()-(k-2)/2.0,k,k,redpen,redBrush));
//           }
//         }


//11111111111111111111111111111111111111111111111111111111111

        double sum_blue = 0.0;                                      // sum blue

       for(int i=0 ; i<spin_polynomial_blue.size();i++)
         {
             sum_blue += spin_polynomial_blue[i];
         }

       double sum_red = 0.0;                                        // sum red

       for(int i=0 ; i<spin_polynomial_red.size();i++)
        {
           sum_red += spin_polynomial_red[i];
        }


       cout << "sum blue = " << sum_blue << "  " << " sum red = " << sum_red <<endl;



            for(int k=0 ; k<spin_polynomial_blue.size();k++)   // divide by sum blue , get blue  precent
            {
               spin_polynomial_blue[k]/=sum_blue ;

            }

            for(int k=0 ; k<spin_polynomial_red.size();k++)   // divide by sum red , get red precent
            {
               spin_polynomial_red[k]/=sum_red ;

            }



       RedMin  = spin_polynomial_red[0];
       BlueMin = spin_polynomial_blue[0];
       for(int k=0 ; k<spin_polynomial_red.size();k++)   // find minimum number in "red" list
       {

             if (spin_polynomial_red[k]<= RedMin )
             {
                RedMin = spin_polynomial_red[k];

             }

       }

       for(int k=0 ; k<spin_polynomial_blue.size();k++)   // find minimum number in "blue" list
       {

             if (spin_polynomial_blue[k]<= BlueMin )
             {
                BlueMin = spin_polynomial_blue[k];

             }

       }

        cout << "Red Min = " << RedMin << endl;
        cout << "Blue Min = " << BlueMin << endl;

        RedMax   = spin_polynomial_red[0] ;
        BlueMax  = spin_polynomial_blue[0] ;

        for(int k=0 ; k<spin_polynomial_red.size();k++)   // find maxinum number in red list
        {
             if(spin_polynomial_red[k]> RedMax)
               {
                 RedMax = spin_polynomial_red[k];

                }

        }

        for(int k=0 ; k<spin_polynomial_blue.size();k++)   // find maxinum number in blue list
        {
             if(spin_polynomial_blue[k]> BlueMax)
               {
                 BlueMax = spin_polynomial_blue[k];

                }

        }


             for(int k=0 ; k<spin_polynomial_red.size();k++)   //  normalize red circle size
             {
                   spin_polynomial_red[k] /= RedMax ;

    //               cout <<"spin poly red " << k << " = "<< spin_polynomial_red[k] << endl ;

             }

             for(int k=0 ; k<spin_polynomial_blue.size();k++)   //  normalize blue circle size
             {
                   spin_polynomial_blue[k] /= BlueMax ;
    //               cout <<"spin poly blue " << k << " = "<< spin_polynomial_blue[k] << endl ;
             }


            for(int i=0; i<spin_polynomial_red.size(); i++)
            {
              double k=25*spin_polynomial_red[i];
              QBrush redBrush(Qt::red);
              QPen redpen(Qt::red);
              QGraphicsEllipseItem *circle =(scene->addEllipse(red_atom_coordinate[i].x()-(k-3)/2.0,red_atom_coordinate[i].y()-(k-3)/2.0,k,k,redpen,redBrush));
              red_circle.append(circle);
           }

            for(int i=0; i<spin_polynomial_blue.size(); i++)
            {
              double k=25*spin_polynomial_blue[i];
              QBrush redBrush(Qt::blue);
              QPen redpen(Qt::blue);
              QGraphicsEllipseItem *circle =(scene->addEllipse(blue_atom_coordinate[i].x()-(k-3)/2.0,blue_atom_coordinate[i].y()-(k-3)/2.0,k,k,redpen,redBrush));
              blue_circle.append(circle);

           }









}


   void ZZSpinpopulaitonCalculatorPanel3::clear_all_things()
   {

       scene->clear();
       red_atom_coordinate.clear();
       blue_atom_coordinate.clear();
       spin_polynomial_blue.clear();
       spin_polynomial_red.clear();
       m_textItems.clear();
       m_textItems2.clear();
       red_circle.clear();
       red_circle2.clear();
       blue_circle.clear();
       blue_circle2.clear();


   }


void ZZSpinpopulaitonCalculatorPanel3::clearText()
{

    for(int i=0; i<m_textItems.size();i++)
    {
        scene->removeItem(m_textItems[i]);

    }


    m_textItems.clear();

    for(int i=0;i<red_circle.size();i++)
    {

        scene->removeItem(red_circle[i]);

    }

    for(int i=0;i<blue_circle.size();i++)
    {

        scene->removeItem(blue_circle[i]);

    }

    red_circle.clear();

    blue_circle.clear();
}
void ZZSpinpopulaitonCalculatorPanel3::clearText2()
{
    for(int i=0; i<m_textItems2.size();++i)
    {
        scene->removeItem(m_textItems2[i]);
    }
    m_textItems2.clear();


    for(int i=0;i<red_circle2.size();i++)
    {

        scene->removeItem(red_circle2[i]);

    }

    for(int i=0;i<blue_circle2.size();i++)
    {

        scene->removeItem(blue_circle2[i]);

    }

    red_circle2.clear();

    blue_circle2.clear();
}






void ZZSpinpopulaitonCalculatorPanel3::resizeEvent(QResizeEvent *)
{
    if (graph_item!=nullptr)
    {
//        view->ensureVisible(graph_item);
        view->centerOn(graph_item);
    }
}

void ZZSpinpopulaitonCalculatorPanel3::list()
{


    clearText2();

    QBrush redBrush(Qt::red);
    QPen redpen(Qt::red);
    QBrush blueBrush(Qt::blue);
    QPen bluepen(Qt::blue);
    QPen blackpen(Qt::white);
    GraphModelGraphicsItem4 *graph_item2 =  new GraphModelGraphicsItem4(m_graph);


     QPointF standard_p ;
     standard_p.setX(graph_item2->x());
     standard_p.setY(graph_item2->y());


    srand(time(NULL));         // set 3 radom numbers

        if(spin_polynomial_red.size() > spin_polynomial_blue.size() )
        {
        Radom1 =  (rand()%spin_polynomial_blue.size() );
        Radom2 =  (rand()%spin_polynomial_blue.size() );
        Radom3 =  (rand()%spin_polynomial_blue.size() );
        }
        else
        {
            Radom1 =  (rand()%spin_polynomial_red.size() );
            Radom2 =  (rand()%spin_polynomial_red.size() );
            Radom3 =  (rand()%spin_polynomial_red.size() );
        }


    cout << Radom1 << " " << Radom2 << " " << Radom3<< endl;

      double red_mid;
      double red_x = max(max(spin_polynomial_red[Radom1],spin_polynomial_red[Radom2]),spin_polynomial_red[Radom3]);
      double red_y = min(min(spin_polynomial_red[Radom1],spin_polynomial_red[Radom2]),spin_polynomial_red[Radom3]);
    {

     if(red_x>=spin_polynomial_red[Radom3] && spin_polynomial_red[Radom3]>=red_y )
     {
      red_mid = spin_polynomial_red[Radom3];
      cout <<"red_mid= " << red_mid << endl ;
     }
    else if(red_x>=spin_polynomial_red[Radom2] && spin_polynomial_red[Radom2]>=red_y )
     {
       red_mid =spin_polynomial_red[Radom2];
       cout <<"red_mid= " << red_mid << endl ;
     }
    else if(red_x>=spin_polynomial_red[Radom1] && spin_polynomial_red[Radom1]>=red_y )
       {
         red_mid = spin_polynomial_red[Radom1];
         cout <<"red_mid= " << red_mid << endl ;
       }

      }


      double blue_mid;
      double blue_x = max(max(spin_polynomial_blue[Radom1],spin_polynomial_blue[Radom2]),spin_polynomial_blue[Radom3]);
      double blue_y = min(min(spin_polynomial_blue[Radom1],spin_polynomial_blue[Radom2]),spin_polynomial_blue[Radom3]);
    {

     if(blue_x>=spin_polynomial_blue[Radom3] && spin_polynomial_blue[Radom3]>=blue_y )
     {
      blue_mid = spin_polynomial_blue[Radom3];
      cout <<"blue_mid= " << blue_mid << endl ;
     }
    else if(blue_x>=spin_polynomial_blue[Radom2] && spin_polynomial_blue[Radom2]>=blue_y )
     {
       blue_mid =spin_polynomial_blue[Radom2];
       cout <<"blue_mid= " << blue_mid << endl ;
     }
    else if(blue_x>=spin_polynomial_blue[Radom1] && spin_polynomial_blue[Radom1]>=blue_y )
       {
         blue_mid = spin_polynomial_blue[Radom1];
         cout <<"blue_mid= " << blue_mid << endl ;
       }

      }

      cout << "Radom_blue_zz1= " << spin_polynomial_blue[Radom1]*BlueMax <<endl;
      cout << "Radom_blue_zz2= " << spin_polynomial_blue[Radom2]*BlueMax <<endl;
      cout << "Radom_blue_zz3= " << spin_polynomial_blue[Radom3]*BlueMax <<endl;

          QList<double> number_blue ;
          QFont font;
          font.setPixelSize(12.0);
          number_blue.append( BlueMax*100 );
          number_blue.append( blue_x*100*BlueMax);
          number_blue.append( blue_mid*100*BlueMax);
          number_blue.append( blue_y*100*BlueMax);
          number_blue.append( BlueMin*100);

          for (int i=0 ; i<5;i++)
         {
              if(number_blue[i]<1 && number_blue[i]!=0)
              {
                  prescentage_blue = QString("<1%");
              }
              else
              {
                  prescentage_blue = QString("%1%").arg(int(number_blue[i]));
              }
              QGraphicsTextItem *text2 = new QGraphicsTextItem(prescentage_blue);
              text2->setFont(font);
              text2->setDefaultTextColor("blue");
              text2->setPos(standard_p.x()-136,standard_p.y()+12+30*i);
              scene -> addItem(text2);
              m_textItems2.append(text2);
         }

          double b1 = 25;
          double b2 = 25*blue_x  ;
          double b3 = 25*blue_mid ;
          double b4 = 25*blue_y  ;
          double b5 = 25*BlueMin/BlueMax;
          blue_circle2.append(scene->addEllipse(graph_item2->x()-158-(b1)/2,graph_item2->y()+15 -(b1/2)  ,b1,b1,bluepen,blueBrush));
          blue_circle2.append(scene->addEllipse(graph_item2->x()-158-(b2)/2,graph_item2->y()+45 -(b2/2)  ,b2,b2,bluepen,blueBrush));
          blue_circle2.append(scene->addEllipse(graph_item2->x()-158-(b3)/2,graph_item2->y()+75 -(b3/2)  ,b3,b3,bluepen,blueBrush));
          blue_circle2.append(scene->addEllipse(graph_item2->x()-158-(b4)/2,graph_item2->y()+105-(b4/2)  ,b4,b4,bluepen,blueBrush));
          blue_circle2.append(scene->addEllipse(graph_item2->x()-158-(b5)/2,graph_item2->y()+135-(b5/2)  ,b5,b5,bluepen,blueBrush));

    cout << "Radom_red_zz1= " << spin_polynomial_red[Radom1]*RedMax <<endl;
    cout << "Radom_red_zz2= " << spin_polynomial_red[Radom2]*RedMax <<endl;
    cout << "Radom_red_zz3= " << spin_polynomial_red[Radom3]*RedMax <<endl;


                   QList<double> number_red ;
                   number_red.append( RedMax*100 );
                   number_red.append( red_x*100*RedMax);
                   number_red.append( red_mid*100*RedMax);
                   number_red.append( red_y*100*RedMax);
                   number_red.append( RedMin*100) ;

                 for (int i=0 ; i<5;i++)
                    {
                     if(number_red[i]<1 && number_red[i]!=0 )
                     {
                          prescentage_red = QString("<1%");
                     }

                     else
                     {
                         prescentage_red = QString("%1%").arg(int(number_red[i]));

                     }
                          QGraphicsTextItem *text = new QGraphicsTextItem(prescentage_red);
                          text->setFont(font);
                          text->setDefaultTextColor("blue");
                          text->setPos(standard_p.x()-68,standard_p.y()+12+30*i);
                          scene -> addItem(text);
                          m_textItems2.append(text);

                    }

                 for(int i=0 ; i<2;i++)
                 {
                     QString Max_prescentage = QString("Max");
                     QString Min_prescentage = QString("Min");
                     QGraphicsTextItem *Max = new QGraphicsTextItem(Max_prescentage);
                     QGraphicsTextItem *Min = new QGraphicsTextItem(Min_prescentage);
                     Max->setFont(font);
                     Min->setFont(font);
                     Max->setDefaultTextColor("green");
                     Min->setDefaultTextColor("green");

                     Max->setPos(standard_p.x()-68*(i+1),standard_p.y());
                     Min->setPos(standard_p.x()-68*(i+1),standard_p.y()+120);

                     scene -> addItem(Max);
                     scene -> addItem(Min);
                     m_textItems2.append(Max);
                     m_textItems2.append(Min);
                 }


     cout << spin_polynomial_red[Radom3]*RedMax << " " << spin_polynomial_red[Radom2]*RedMax <<" " <<spin_polynomial_red[Radom1]*RedMax << endl;


                   double k1 = 25;
                   double k2 = 25*red_x  ;
                   double k3 = 25*red_mid ;
                   double k4 = 25*red_y  ;
                   double k5 = 25*RedMin/RedMax;
                   red_circle2.append(scene->addEllipse(graph_item2->x()-90-(k1)/2,graph_item2->y()+15 -(k1/2)  ,k1,k1,redpen,redBrush));
                   red_circle2.append(scene->addEllipse(graph_item2->x()-90-(k2)/2,graph_item2->y()+45 -(k2/2)  ,k2,k2,redpen,redBrush));
                   red_circle2.append(scene->addEllipse(graph_item2->x()-90-(k3)/2,graph_item2->y()+75-(k3/2)   ,k3,k3,redpen,redBrush));
                   red_circle2.append(scene->addEllipse(graph_item2->x()-90-(k4)/2,graph_item2->y()+105-(k4/2)  ,k4,k4,redpen,redBrush));
                   red_circle2.append(scene->addEllipse(graph_item2->x()-90-(k5)/2,graph_item2->y()+135-(k5/2)  ,k5,k5,redpen,redBrush));

}



void ZZSpinpopulaitonCalculatorPanel3::save_connected_edges()
{
//for(int i=0 ; i<graph_item2->atom_coordinate.size();i++)
//{
//    cout<<graph_item2->atom_coordinate[i].x() << "  " << graph_item2->atom_coordinate[i].y()<<endl;

//}
}
