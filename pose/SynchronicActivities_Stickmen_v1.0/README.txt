Synchronic Activities Stickmen V 1.0
====================================

M. Eichner and V. Ferrari
 
 
Introduction: a dataset of photos of synchronic activities for human pose estimation
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 
Welcome to this release of the Synchronic Activities Stickmen dataset! This dataset focuses on scenarios where multiple persons perform some activity synchronously. This happens often in sports (gymnastics, diving) or when a group of people exercises following a leader (aerobic, cheerleading, dancing, martial arts). We release here a dataset of such synchronic activity photos of dancing, aerobic and cheer-leading, complete with annotations of the six upper body parts of all persons performing the activity. The dataset has 357 images sampled from synchronic activities videos downloaded from Youtube.

In each image we have annotated the upper-body of every approximately upright and frontal person, for a total of 1128 persons (on average 3 persons per image). A body part is annotated by a line segment. The parts annotated are head, torso, upper and lower arms. The 6 annotated line segments for a person constitute a 'stickman'. Results on this dataset have been first published in [1]. Please cite it if you use this dataset.
 
 
Content
~~~~~~~

Let <dir_root> be the directory where this package was uncompressed.

This package contains:

 - 357 raw images in <dir_root>/images

 - corresponding ground-truth stickmen annotations (referred to as 'GT stickmen' from now on) in <dir_root>/data/SynchronicActivities.txt. There is a single annotation text file covering all 357 images.

 - matlab code to read and visualize GT stickmen in <dir_root>/code

 - matlab code to evaluate stickmen estimated by an algorithm against GT stickmen in <dir_root>/code

 - human pose estimation results from [1]. These are all stickmen estimated by our Hierarchical Pose Co-Estimation model from [1] (Hierarchical PCE) based on the detection windows from our upper-body detector [3]. These detection windows are also included in this release WHERE?

 - PCP performance curves from [1] (see below for details about PCP curves)

 - Dataset Stickmen Distribution (inspired by [9])

 - images with stickmen overlaid in <dir_root>/overlays. These overlays are useful for surfing the dataset and for checking whether you have read the annotation text files correctly.

 
 
Installation
~~~~~~~~~~~~
 
You can follow the next steps to check that everything is working properly:
 
1) start matlab 
 
2) navigate to <dir_root>/code (e.g. by using the 'cd' command)
  
3) execute command: startup
   This will add necessary paths to your matlab environment
 
4) if this is the first time you run the code, then execute installmex.
   This will compile the mex-files for your system.
 
5) execute the following to display the first GT stickman from the first annotated image:

        lF = ReadStickmenAnnotationTxtMulti('../data/SynchronicActivities.txt');
        img = imread(fullfile('../images/',lF(1).filename ));
        hdl = DrawStickman(lF(1).stickmen(1).coor, img);
   
   check that a new figure is open with the same content as
   '<dir_root>/code/Dirty_Dancing_Fitness_Workout_00000051.jpg'
 
6) execute the following commands to recompute our best result from [1]:
 
        % loading ground-truth annotations
        GT = ReadStickmenAnnotationTxtMulti('../data/SynchronicActivities.txt');
        % loading stickmen for the evaluation
        load('../pami12_SynchronicActivities_Results.mat');
        % evaluating best multi-person system (hierarchical PCE) [1]
        [detrate PCP] = BatchEval(@detBBFromStickmanSynchro,@EvalStickmenMulti,pami12_hPCE_synchro,GT)
 
  You should obtain detRate = 0.8422, PCP = 0.7951
 
7) reproduce the PCP performance curve for our Hierarchical PCE model from [1]:
        
        calcPCPcurve(@detBBFromStickmanSynchro,@EvalStickmenMulti,pami12_hPCE_synchro,GT,[],true,[0 1 0]);
 
 You should obtain the same curve as the top curve in the figure:
 <dir_root>/PCP_pami12_SynchronicActivities_Results.fig (details below)
 
8) if all above points went well, this package is working perfectly.
 
 
Description of the annotation files
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 
Each text file in <dir_root>/data contains annotations in the format:
 
<file_name_i> Nstickmen Nparts
<1x11>  <1y11> <1x12> <1y12> 
<1x21>  <1y21> <1x22> <1y22> 
<1x31>  <1y31> <1x32> <1y32> 
<1x41>  <1y41> <1x42> <1y42> 
<1x51>  <1y51> <1x52> <1y52> 
<1x61>  <1y61> <1x62> <1y62> 
<2x11>  <2y11> <2x12> <2y12>
<2x21>  <2y21> <2x22> <2y22> 
<2x31>  <2y31> <2x32> <2y32> 
<2x41>  <2y41> <2x42> <2y42> 
<2x51>  <2y51> <2x52> <2y52> 
<2x61>  <2y61> <2x62> <2y62>  
...
<file_name_i+1> Nstickmen Nparts
...
 
where:
- <file_name_i> is the filename of the image i
- Nstickmen is the number of GT stickmen annotated in the current image
- Nparts is always 6
- each <ncsp> is a coordinate of a body part of a stickmen. More precisely:
    - stickman n (from 1 to Nstickmen)
    - coordinate c (x or y)
    - stick s (from 1 to 6)
    - end point p (1 or 2)
  The index s correspond to 1-> torso, 2-> left upper arm, 3-> right upper arm, 4-> left lower arm, 5-> right lower arm, 6-> head ('left' and 'right' as they appear in the image).
  If <nxs1> <nys1> <nxs2> <nys2> == NaN NaN NaN NaN then stick s of GT stickman n is occluded. (see the code in ‘me_isEmptyStick.m’) 
 
 
Matlab code
~~~~~~~~~~~
 
The following Matlab functions are provided:
- ReadStickmenAnnotationTxtMulti: reads an annotation file
- DrawStickman: draws a selected GT stickman for a single image
- EvalStickmenMulti: evaluate all estimated stickmen for an image against the GT stickmen for that image
- BatchEval: evaluate multiple images
- detBBFromDetectionSynchro - function generating a detection window from a stickman (dataset  specific)
 
For exact input/output arguments format please check the function source code.
 
 
Evaluation criterion [2]
~~~~~~~~~~~~~~~~~~~~~~~~
 
We use the evaluation criterion of [2] (used also in [2,4,5,6,7]). The criterion was previously defined to evaluate human pose estimation performance on the Buffy Stickmen and ETHZ Pascal Stickmen datasets [8].

For each image, the routine BatchEval expects your system to provide a set of
detected persons. Each detected person consists of an estimated window
around the head and shoulders, as well as an estimated stickman. If you
only provide the stickman, BatchEval will derive such a window from the estimated stickman.
 
Given this input, BatchEval will compute two numbers:
 
a) Detection rate
indicates how many of the GT stickmen have been detected.
A GT stickmen is counted as detected if the estimated window provided by your system covers a GT window, which is automatically derived from the GT stickman. More precisely, the Intersection area divided by the Union area of two windows (IoU) must be greater than 0.5 (the PASCAL VOC criterion).
 
b) PCP (Percentage of Correctly estimated body Parts)
an estimated body part is counted as correct if its segment endpoints lie within a fraction of the length (pcp-threshold) of the ground-truth segment from their annotated location. 
A body part estimated as occluded is considered correct only if it is occluded also in the ground-truth annotation.
PCP is evaluated only for the GT stickmen for which your system produced a correct detection window (see a).
 
To obtain the total PCP over the whole test set, not only over images with a correct detection
window, please multiply PCP by the detection rate (i.e. multiply the two numbers output by BatchEval).
 

Clarification of the PCP evaluation criterion
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The matlab code to evaluate PCP provided with this dataset represents the official evaluation protocol for the following datasets: Buffy Stickmen, ETHZ PASCAL Stickmen, We Are Family Stickmen and Synchronic Activities Stickmen. In our PCP implementation, a body part produced by an algorithm is considered correctly localized if its endpoints are closer to their ground-truth locations than a threshold (on average over the two endpoints). Using it ensures results comparable to the vast majority of results previously reported on these dataset.

Recently an alternative implementation of the PCP criterion, based on a stricter interpretation of its description in Ferrari et al CVPR 2008 has been used in some works, including Johnson et al. BMVC 2010 and Pishchulin et al CVPR 2012. In this implementation, a body part is considered correct only if both of its endpoints are closer to their ground-truth locations than a threshold. These two different PCP measures are the consequence of the ambiguous wording in the original verbal description of PCP in Ferrari et al CVPR 2008 (which did not mention averaging over endpoints). Importantly, the stricter PCP version has essentially been used only on other datasets than the ones mentioned above, and in particular on IIP (Iterative Image Parsing dataset, Ramanan NIPS 2006) and LSP (Leeds Sports Pose dataset, Johnson et al. BMVC 2010).

In order to keep a healthy research environment and guarantee the comparability of results across different research groups and different years, we recommend the following policy:
- use our evaluation code, which computes the original, looser PCP measure, on Buffy Stickmen, ETHZ PASCAL Stickmen, We Are Family Stickmen, Synchronic Activities Stickmen, i.e. essentially on all datasets released by us
- some other datasets unfortunately have no official evaluation code released with them, and therefore it is harder to establish an exact and fully official protocol. Nonetheless, based on the protocols followed by most papers that appeared so far, we recommend using the strict PCP measure on IIP and LSP. A precise definition of the strict PCP measure can be found in Pishchulin et al CVPR 2012.

D. Ramanan. "Learning to Parse Images of Articulated Objects", In NIPS, 2006.
S. Johnson and M. Everingham "Clustered Pose and Nonlinear Appearance Models for Human Pose Estimation", In BMVC, 2010 
L. Pishchulin, A. Jain, M. Andriluka, T. Thormaehlen and B. Schiele "Articulated People Detection and Pose Estimation: Reshaping the Future", In CVPR, 2012


Performance of [1]
~~~~~~~~~~~~~~~~~~
For convenience we provide a figure containing PCP performance curves from [1].
The curves are release in PNG format (<dir_root>/PCP_pami12_SynchronicActivities_Results.png) as well as Matlab figures (<dir_root>/PCP_pami12_SynchronicActivities_Results.fig).
The curves were obtained by varying the PCP threshold (horizontal axis). The PCP curve enables to observe how well a system does as the definition of 'correctly estimated' body part gets tighter. You can reproduce this curve with the calcPCPcurve routine.
 
 
Outputs of [1]
~~~~~~~~~~~~~~
We also provide data structures containing the outputs of all models presented in [1] on this dataset:

system name         variablename
---------------------------------------
Hierarchical PCE   pami12_hPCE_synchro
comboPS            pami12_comboPS_synchro
[2,4]              pami12_Eichner09bmvc_synchro
[5]                pami12_Andriluka09cvpr_synchro
[7]                pami12_Sapp10eccv_synchro

They are stored in this mat-file:
 
<dir_root>/pami12_SynchronicActivities_Results.mat
 
Load them in Matlab (version 7 or later) by executing this command:
 
load('<dir_root>/pami12_SynchronicActivities_Results.mat')
 
Results are provided for in the following format:
 
pami12_<systemname>_synchro: (1x357 struct)
    .filename: image filename
    .stickmen: (1xD struct) of results for .filename (D = number of detection windows in this image)
        .coor: 4x6 array of sticks coordinates estimated by <systemname> in the same order as GT (see above)
        .det: [minx miny maxx maxy] coordinates of the detection window (the same in all <systemname>)
 
This data can be used to reproduce the performance curves (using the calcPCPcurve routine).

The detection windows we release here were obtained by running an advanced person detector [3], which combines several independently trained detectors. After running the detector on each frame of the original videos separately, we removed false positives by 1) joining individual detections into tracks using a variety of cues such as bounding-box overlap, appearance similarity and KLT-tracks overlap; 2) discarding tracks shorter than 12 frames. As post-processing we smooth track trajectories and fill gaps within each track by linear interpolation. You might want to use these detection windows inside your own human pose estimation pipeline to ensure an exact comparison to [1] in terms of PCP.


Stickmen Distribution of the Dataset
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

We also provide a stickmen distribution plot inspired by [9] depicting articulation variability in a dataset (SynchronicActivities_stickmen_distribution.*). 
This plot is obtained by first scale-normalizing all GT stickmen in the dataset, then aligning them at the jugular notch, and finally superimposing them all.

 
Support
~~~~~~~
 
For any query/suggestion/complaint or simply to say you like/use the annotation and software just drop us an email
 
eichner@vision.ee.ethz.ch
vferrari@staffmail.ed.ac.uk
 
 
References
~~~~~~~~~~

[1] Human Pose Co-Estimation and Applications
M. Eichner and V. Ferrari, 
IEEE Transactions on Pattern Analysis and Machine Intelligence (PAMI), (to appear)

[2] 2D articulated Human Pose Estimation and Retrieval in (Almost) Unconstrained Still Images
M. Eichner, M. Marin-Jimenez, A. Zisserman, V.Ferrari 
International Journal of Computer Vision (IJCV), April 2012.

[3] Weakly supervised learning of interactions between humans and objects
A. Prest, C. Schmid and V. Ferrari,
IEEE Transactions on Pattern Analysis and Machine Intelligence (PAMI), March 2012
 
[4] Better appearance models for pictorial structures
M. Eichner and V. Ferrari
British Machine Vision Conference (BMVC), September 2009.
 
[5] Pictorial Structures Revisited: People Detection and Articulated Pose Estimation
M. Andriluka, S. Roth, B. Schiele.
IEEE Computer Vision and Pattern Recognition (CVPR), June 2009.
 
[6] Adaptive Pose Priors for Pictorial Structures
B. Sapp, C. Jordan and B. Taskar.
IEEE Computer Vision and Pattern Recognition (CVPR), June 2010.
 
[7] Cascaded Models for Articulated Pose Estimation
B. Sapp, C. Jordan and B. Taskar.
European Conference in Computer Vision (ECCV), June 2010.
 
[8] http://www.vision.ee.ethz.ch/~calvin/datasets.html
 
[9] Improved Human Parsing with a Full Relational Model 
D. Tran and D. Forsyth 
European Conference in Computer Vision (ECCV), June 2010.
 

Version History
~~~~~~~~~~~~~~~
 
Version 1.0 (August 2012)
-----------
- initial release
