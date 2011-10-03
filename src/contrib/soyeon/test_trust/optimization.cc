#include "optimization.h"
#include "math.h"
#include <iostream>
#include <algorithm>	//max
using namespace std;	//max
 

void Optimization::Init(fx_module *module) {
	module_=module;
	max_radius_=10.0;
}


void Optimization::ComputeDoglegDirection(double radius, 
																					Vector &gradient,
																					Matrix &hessian,
																					Vector *p,
																					double *delta_m) {
	//check positive definiteness of the hessian
	Matrix inverse_hessian;
	if( !(la::InverseInit(hessian, &inverse_hessian)) ) {

		cout<<"Hessian matrix is not invertible"<<endl;
		cout<<"Find Cauchy Point..."<<endl;

		//if hessian is indefinite->use cauchy point
		double gHg;

		Matrix transpose_hessian;
		la::TransposeInit(hessian, &transpose_hessian);

		Vector temp2; //H*g
		la::MulInit(hessian, gradient, &temp2);
		gHg=la::Dot(temp2, gradient);

		double gradient_norm=sqrt(la::Dot(gradient, gradient));
		

		if(gHg<=0){
			//double gradient_norm=sqrt(la::Dot(gradient, gradient));
			//p=-(radius/gradient_norm)*gradient;
			la::ScaleInit(-1*radius/gradient_norm, gradient, p);
		}
		else{
			double zeta=0;
			zeta=std::min(pow(gradient_norm, 3)/(radius*gHg), 1.0);
			la::ScaleInit(-1*zeta*radius/gradient_norm, gradient, p);

		}
		

	}	//if
	else {	//if hessian matrix is positive definite
		//la::InverseInit(hessian, &inverse_hessian);
		Vector p_b;
		//p_b= - (hessian)^-1 * g
		la::MulInit(inverse_hessian, gradient, &p_b);
		la::Scale(-1.0, &p_b);

		double p_b_norm;
		//p_b_norm=la::Dot(p_b, p_b);
		p_b_norm=sqrt(la::Dot(p_b, p_b));

		if(radius>=p_b_norm){
			p->Copy(p_b);
		}
		else{
			//g'*H*g = (H*g)'g
			double gHg;

			Matrix transpose_hessian;
			la::TransposeInit(hessian, &transpose_hessian);

			/*
			//check whether hessian is symmetric
			double cnt=0;
			for(size_t i=0; i<hessian.n_rows(); i++) {
				for(size_t j=0; j<hessian.n_cols(); j++) {
					if(hessian.get(i,j) != transpose_hessian.get(i,j)){
						cnt+=1;
					}	//if
				}	//j
			}		//i
			if( cnt !=0 ) {
				NOTIFY("Hessian matrix is NOT symmetric.");
			}
			*/
			
			Vector temp1; //H*g
			la::MulInit(hessian, gradient, &temp1);
			gHg=la::Dot(temp1, gradient);

			Vector p_u;
			//p_u= -(g'g/g'Hg)*g
			double p_u_norm;
			la::ScaleInit(-1*la::Dot(gradient, gradient)/gHg, gradient, &p_u);
			p_u_norm=sqrt(la::Dot(p_u, p_u));

			if( p_u_norm>=radius ) {	//p=radius/p_u_norm * p_u)
				la::ScaleInit(radius/p_u_norm, p_u, p);
			}	//if
			else{	//combination of p_u and p_b
				//solve the quadratic equation ||p_u-zeta(p_b-p_u)||^2=radius^2
				Vector diff; //p_b-p_u
				la::SubInit(p_u, p_b, &diff);
				double a=la::Dot(diff, diff);
				Vector diff2; //2p_u-p_b
				la::ScaleInit(2, p_u, &diff2);
				la::SubFrom(p_b, &diff2);
				double b=la::Dot(diff, diff2);
				double c=la::Dot(diff2, diff2)-math::Sqr(radius);

				//DEBUG_ASSERT_MSG(b*b-4*a*c>0, 
				//"(Dogleg)Discriminant is negative. Fail to get the solution zeta.");
				if(b*b-4*a*c<=0){
					NOTIFY("(Dogleg)Discriminant is negative. Fail to get the solution zeta.");
					//double sqrt_discriminant =sqrt(-1*(b*b-4*a*c));
					NOTIFY("Use cauchy point");
					//if hessian is indefinite->use cauchy point
					double gHg;

					Matrix transpose_hessian;
					la::TransposeInit(hessian, &transpose_hessian);

					Vector temp2; //H*g
					la::MulInit(hessian, gradient, &temp2);
					gHg=la::Dot(temp2, gradient);

					double gradient_norm=sqrt(la::Dot(gradient, gradient));
					

					if(gHg<=0){
						//double gradient_norm=sqrt(la::Dot(gradient, gradient));
						//p=-(radius/gradient_norm)*gradient;
						la::ScaleInit(-1*radius/gradient_norm, gradient, p);
					}
					else{
						double zeta=0;
						zeta=std::min(pow(gradient_norm, 3)/(radius*gHg), 1.0);
						la::ScaleInit(-1*zeta*radius/gradient_norm, gradient, p);

					}
		
				}
				else if(b*b-4*a*c>0) {
					double sqrt_discriminant =sqrt(b*b-4*a*c);
				
					double zeta1=(-b+sqrt_discriminant)/(2*a);
					double zeta2=(-b-sqrt_discriminant)/(2*a);
					double zeta=-1;

					if( (zeta1<2)&&(zeta1>0)&&(zeta2<2)&&(zeta2>0)){
						zeta=max(zeta1, zeta2);
					}
					else if( (zeta1<2)&&(zeta1>0) ){
						zeta=zeta1;
					}
					else if((zeta2<2)&&(zeta2>0)){
						zeta=zeta2;
					}
					else{
						//DEBUG_ASSERT_MSG((zeta>0), "Fail to get zeta");
						NOTIFY("Fail to get zeta");
						zeta=0.5;
					}

					if(zeta<=1){
						la::ScaleInit(zeta, p_u, p);
					}
					else{
						Vector temp; //(zeta-1)*(p_b-p_u)
						la::ScaleInit((zeta-1), diff, &temp);
						la::AddInit(p_u, temp, p);
					}		//else
				
				}	//else
			}
	}
	}

	//delta_m calculation -g'p-0.5*p'Hp=-g'p-0.5*(Hp)'p
	Vector temp3; //Hp
	la::MulInit(hessian, *p, &temp3);
	double pHp=0;;
	pHp=la::Dot(temp3, *p);

	*delta_m=-1*(la::Dot(gradient, *p))-0.5*(pHp);
	
}




void Optimization::ComputeScaledDoglegDirection(double radius, 
																					Vector &gradient,
																					Matrix &hessian,
																					Vector *p,
																					double *delta_m) {
	//check positive definiteness of the hessian
	Matrix inverse_hessian;
	if( !(la::InverseInit(hessian, &inverse_hessian)) ) {

		cout<<"Hessian matrix is not invertible"<<endl;
		cout<<"Find Cauchy Point..."<<endl;

		//if hessian is indefinite->use cauchy point
		double gHg;

		Matrix transpose_hessian;
		la::TransposeInit(hessian, &transpose_hessian);

		Vector temp2; //H*g
		la::MulInit(hessian, gradient, &temp2);
		gHg=la::Dot(temp2, gradient);

		double gradient_norm=sqrt(la::Dot(gradient, gradient));
		

		if(gHg<=0){
			//double gradient_norm=sqrt(la::Dot(gradient, gradient));
			//p=-(radius/gradient_norm)*gradient;
			la::ScaleInit(-1*radius/gradient_norm, gradient, p);
		}
		else{
			double zeta=0;
			zeta=std::min(pow(gradient_norm, 3)/(radius*gHg), 1.0);
			la::ScaleInit(-1*zeta*radius/gradient_norm, gradient, p);

		}
		

	}	//if
	else {	//if hessian matrix is positive definite
		//la::InverseInit(hessian, &inverse_hessian);
		Vector p_b;
		//p_b= - (hessian)^-1 * g
		la::MulInit(inverse_hessian, gradient, &p_b);
		la::Scale(-1.0, &p_b);

		double p_b_norm;
		//p_b_norm=la::Dot(p_b, p_b);
		p_b_norm=sqrt(la::Dot(p_b, p_b));

		if(radius>=p_b_norm){
			p->Copy(p_b);
		}
		else{
			//g'*H*g = (H*g)'g
			double gHg;

			Matrix transpose_hessian;
			la::TransposeInit(hessian, &transpose_hessian);

			/*
			//check whether hessian is symmetric
			double cnt=0;
			for(size_t i=0; i<hessian.n_rows(); i++) {
				for(size_t j=0; j<hessian.n_cols(); j++) {
					if(hessian.get(i,j) != transpose_hessian.get(i,j)){
						cnt+=1;
					}	//if
				}	//j
			}		//i
			if( cnt !=0 ) {
				NOTIFY("Hessian matrix is NOT symmetric.");
			}
			*/
			Matrix current_inverse_hessian;
			if( !(la::InverseInit(hessian, &current_inverse_hessian)) ) {
				NOTIFY("Current hessian matrix is not invertible!");
			}
		


			
			Vector temp1; //H*g
			la::MulInit(hessian, gradient, &temp1);
			gHg=la::Dot(temp1, gradient);

			Vector p_u;
			//p_u= -(g'g/g'Hg)*g
			//Scaled version
			//p_u=-(g'(H^-1g)/g'Hg)*(H^-1g)
			double p_u_norm;
			//la::ScaleInit(-1*la::Dot(gradient, gradient)/gHg, gradient, &p_u);
			Vector scaled_gradient;
			la::MulInit(inverse_hessian, gradient, &scaled_gradient);
			la::ScaleInit(-1*la::Dot(gradient, scaled_gradient)/gHg, scaled_gradient, &p_u);
			p_u_norm=sqrt(la::Dot(p_u, p_u));

			if( p_u_norm>=radius ) {	//p=radius/p_u_norm * p_u)
				la::ScaleInit(radius/p_u_norm, p_u, p);
			}	//if
			else{	//combination of p_u and p_b
				//solve the quadratic equation ||p_u-zeta(p_b-p_u)||^2=radius^2
				Vector diff; //p_b-p_u
				la::SubInit(p_u, p_b, &diff);
				double a=la::Dot(diff, diff);
				Vector diff2; //2p_u-p_b
				la::ScaleInit(2, p_u, &diff2);
				la::SubFrom(p_b, &diff2);
				double b=la::Dot(diff, diff2);
				double c=la::Dot(diff2, diff2)-math::Sqr(radius);

				//DEBUG_ASSERT_MSG(b*b*-4*a*c>0, 
				//"(Dogleg)Discriminant is negative. Fail to get the solution zeta.");
				if(b*b-4*a*c<=0){
					NOTIFY("(Dogleg)Discriminant is negative. Fail to get the solution zeta.");
					NOTIFY("Use cauchy point");
					double gHg;

					Matrix transpose_hessian;
					la::TransposeInit(hessian, &transpose_hessian);

					Vector temp2; //H*g
					la::MulInit(hessian, gradient, &temp2);
					gHg=la::Dot(temp2, gradient);

					double gradient_norm=sqrt(la::Dot(gradient, gradient));
					

					if(gHg<=0){
						//double gradient_norm=sqrt(la::Dot(gradient, gradient));
						//p=-(radius/gradient_norm)*gradient;
						la::ScaleInit(-1*radius/gradient_norm, gradient, p);
					}
					else{
						double zeta=0;
						zeta=std::min(pow(gradient_norm, 3)/(radius*gHg), 1.0);
						la::ScaleInit(-1*zeta*radius/gradient_norm, gradient, p);

					}
				}
				else if(b*b-4*a*c>0) {
					double sqrt_discriminant=sqrt(b*b-4*a*c);
					double zeta1=(-b+sqrt_discriminant)/(2*a);
					double zeta2=(-b-sqrt_discriminant)/(2*a);
					double zeta=-1;

					if( (zeta1<2)&&(zeta1>0)&&(zeta2<2)&&(zeta2>0)){
						zeta=max(zeta1, zeta2);
					}
					else if( (zeta1<2)&&(zeta1>0) ){
						zeta=zeta1;
					}
					else if((zeta2<2)&&(zeta2>0)){
						zeta=zeta2;
					}
					else{
						//DEBUG_ASSERT_MSG((zeta>0), "Fail to get zeta");
						NOTIFY("Fail to get zeta..use 0.5");
						zeta=0.5;
					}

					if(zeta<=1){
						la::ScaleInit(zeta, p_u, p);
					}
					else{
						Vector temp; //(zeta-1)*(p_b-p_u)
						la::ScaleInit((zeta-1), diff, &temp);
						la::AddInit(p_u, temp, p);
					}		//else
				
				}	//else
			}
		}
	}

	//delta_m calculation -g'p-0.5*p'Hp=-g'p-0.5*(Hp)'p
	Vector temp3; //Hp
	la::MulInit(hessian, *p, &temp3);
	double pHp=0;;
	pHp=la::Dot(temp3, *p);

	*delta_m=-1*(la::Dot(gradient, *p))-0.5*(pHp);
	
}

void Optimization::ComputeSteihaugDirection(double radius, 
																Vector &gradient,
																Matrix &hessian,
																Vector *p,
																double *delta_m
																) {
	//"Numerical optimization" p.171 CG-Steihaug
	//Truncated conjugated gradient algorithm implementation
	//"Trust_Region Methods", pp. 202-207
	//															
	Vector z;
	z.Init(gradient.length());
	z.SetZero();	//z_0=0;

	Vector r;
	r.Alias(gradient);	//r_0=gradient

	Vector old_r;
	old_r.Init(gradient.length());
	old_r.SetZero();

	Vector d;
	la::ScaleInit(-1.0, r, &d);

	//double ZERO_EPS=1e-9;
	double r0_norm;
	r0_norm=sqrt(la::Dot(r,r));
	//Define epsilon
	double e=sqrt(r0_norm);
	if(e>0.1){
		e=0.1;
	}

	Vector temp1;  //Hd
  temp1.Init(gradient.length());

	Vector temp2; //alpha*d
	temp2.Init(gradient.length());

	Vector temp3; //Hd
	temp3.Init(gradient.length());

	Vector temp4; //beta*d
	temp4.Init(gradient.length());

	double cnt=0;
	while(1){
		cnt+=1;
		if(cnt>150){
			NOTIFY("Exceeded the maximum iteration for SteihaugDirection");
			break;
		}
		//Calculate d'Hd=(Hd)'d
		//Vector temp1;  //Hd
		//temp1.Init(gradient.length());
		la::MulOverwrite(hessian, d, &temp1);
		double dHd;
		dHd=la::Dot(temp1, d);

		if(dHd<=0) {
			//find zeta ||p_k||=radius
			double a=la::Dot(d,d);
			double b=2*la::Dot(z,d);
			double c=la::Dot(z,z)-pow(radius,2);
			DEBUG_ASSERT_MSG(b*b*-4*a*c>0, 
				"Discriminant is negative. Fail to get the solution zeta.");
			double sqrt_discriminant=sqrt(b*b-4*a*c);
			double zeta=(-b+sqrt_discriminant)/(2*a);
			
			//p=z+zeta*d
			la::ScaleInit(zeta, d, p);
			la::AddTo(z, p);
			break;
		}

		Vector z_next;
		z_next.Init(z.length());

		double alpha=la::Dot(r,r)/dHd;
		//Vector temp2; //alpha*d
		//temp2.Init(gradient.length());
		la::ScaleOverwrite(alpha, d, &temp2);
		la::AddOverwrite(z, temp2, &z_next);	//z_(j+1)=z_j+alpha*d_j

		if(la::Dot(z_next, z_next)>=(radius*radius)) {
			double a=la::Dot(d,d);
			double b=2*la::Dot(z,d);
			double c=la::Dot(z,z)-pow(radius,2);
			DEBUG_ASSERT_MSG(b*b*-4*a*c>0, 
				"(Second)Discriminant is negative. Fail to get the solution zeta.");
			double sqrt_discriminant=sqrt(b*b-4*a*c);
			double zeta=(-b+sqrt_discriminant)/(2*a);
			
			//p=z+zeta*d
			la::ScaleInit(zeta, d, p);
			la::AddTo(z, p);
			break;
		}
		z.CopyValues(z_next);
		old_r.CopyValues(r);

		//Vector temp3; //Hd
		//temp3.Init(gradient.length());
		la::MulOverwrite(hessian, d, &temp3);
		la::Scale(alpha, &temp3);
		la::AddOverwrite(temp3, old_r, &r);

		if(sqrt(la::Dot(r,r))>r0_norm*e){
			p->Copy(z);
			break;
		}

		double beta=la::Dot(r,r)/la::Dot(old_r, old_r);
		
		//Vector temp4; //beta*d
		//temp4.Init(gradient.length());
		la::ScaleOverwrite(beta, d, &temp4);
		la::SubOverwrite(r, temp4, &d);

		
	
	}	//while

	cout<<"cnt="<<cnt<<endl;
	//Calculate delta_m
	Vector temp5;	//Hp
	la::MulInit(hessian, *p, &temp5);
	double pHp=0;
	pHp=la::Dot(temp5, *p);
	*delta_m=-1*(la::Dot(gradient, *p))-0.5*(pHp);



}



void Optimization::TrustRadiusUpdate(double rho, double p_norm, 
																		 double *current_radius) {
																			 
	if(rho<0.25)
	{
		cout<<"Shrinking trust region radius..."<<endl;
		(*current_radius)=p_norm/4.0; //(*radius)=(*radius)/4.0;
	}
	else if( (rho>0.75) && (p_norm > (0.99*(*current_radius))) )
	{
		cout<<"Expanding trust region radius..."<<endl;
		(*current_radius)=min(2.0*(*current_radius),max_radius_);
	}
}


void Optimization::ComputeDerectionUnderConstraints(double radius, 
																					Vector &gradient,
																					Matrix &hessian,
																					Vector &current_parameter,
																					Vector *p,
																					double *delta_m,
																					Vector *next_parameter,
																					double *new_radius) {

		double constraints_check=1;
		Vector candidate_p;
		double candidate_delta_m;
		Vector candidate_next_parameter;
		candidate_next_parameter.Init(current_parameter.length());

		double candidate_radius=radius;

		
		//else{
		//	cout<<"Diagonal of inverse hessian: ";
		//	for(size_t i=0; i<current_inverse_hessian.n_rows(); i++){
		//		cout<<current_inverse_hessian.get(i,i)<<" ";
		//	}
		//	cout<<endl;
		//}


		while(constraints_check>0){

		
		ComputeDoglegDirection(candidate_radius, gradient, hessian, &candidate_p, &candidate_delta_m);
		//double p_norm=0;
				

			
			//cout<<"p="<<" ";
			//for(size_t i=0; i<current_p.length(); i++){
			//	cout<<current_p[i]<<" ";
			//}
			//cout<<endl;
			
			//cout<<"delta_m="<<candidate_delta_m<<endl;

			
			la::AddOverwrite(candidate_p, current_parameter, &candidate_next_parameter);

			if( candidate_next_parameter[candidate_next_parameter.length()-2]>0 && 
				candidate_next_parameter[candidate_next_parameter.length()-1]>0 ) {
				NOTIFY("Satisfy Constraints");
				p->Copy(candidate_p);
				next_parameter->Copy(candidate_next_parameter);
				(*delta_m)=candidate_delta_m;
				(*new_radius)=candidate_radius;
				constraints_check=-1;

			}
			else{
			cout<<"bed_new_parameter=";
			for(size_t i=0; i<candidate_next_parameter.length(); i++){
				cout<<candidate_next_parameter[i]<<" ";
			}
			cout<<endl;
				
			NOTIFY("Constraint Violation...Shrink Trust region Radius...");
			(candidate_radius)=0.5*candidate_radius;
			}
			/*
			NOTIFY("Projection");
			if(next_parameter[next_parameter.length()-2]<=0) {
				NOTIFY("p is negative");
				next_parameter[next_parameter.length()-2]=0;
			}
			else if(next_parameter[next_parameter.length()-1]<=0) {
				NOTIFY("q is negative");
				next_parameter[next_parameter.length()-1]=0;
			}
			*/
								
			
		}	//while

}


void Optimization::ComputeScaledDerectionUnderConstraints(double radius, 
																					Vector &gradient,
																					Matrix &hessian,
																					Vector &current_parameter,
																					Vector *p,
																					double *delta_m,
																					Vector *next_parameter,
																					double *new_radius) {

		double constraints_check=1;
		Vector candidate_p;
		double candidate_delta_m;
		Vector candidate_next_parameter;
		candidate_next_parameter.Init(current_parameter.length());

		double candidate_radius=radius;

		
		//else{
		//	cout<<"Diagonal of inverse hessian: ";
		//	for(size_t i=0; i<current_inverse_hessian.n_rows(); i++){
		//		cout<<current_inverse_hessian.get(i,i)<<" ";
		//	}
		//	cout<<endl;
		//}


		while(constraints_check>0){

		
		ComputeScaledDoglegDirection(candidate_radius, gradient, hessian, &candidate_p, &candidate_delta_m);
		//double p_norm=0;
				

			
			//cout<<"p="<<" ";
			//for(size_t i=0; i<current_p.length(); i++){
			//	cout<<current_p[i]<<" ";
			//}
			//cout<<endl;
			
			//cout<<"delta_m="<<candidate_delta_m<<endl;

			
			la::AddOverwrite(candidate_p, current_parameter, &candidate_next_parameter);

			if( candidate_next_parameter[candidate_next_parameter.length()-2]>0 && 
				candidate_next_parameter[candidate_next_parameter.length()-1]>0 ) {
				NOTIFY("Satisfy Constraints");
				p->Copy(candidate_p);
				next_parameter->Copy(candidate_next_parameter);
				(*delta_m)=candidate_delta_m;
				(*new_radius)=candidate_radius;
				constraints_check=-1;

			}
			else{
			cout<<"bed_new_parameter=";
			for(size_t i=0; i<candidate_next_parameter.length(); i++){
				cout<<candidate_next_parameter[i]<<" ";
			}
			cout<<endl;
				
			NOTIFY("Constraint Violation...Shrink Trust region Radius...");
			(candidate_radius)=0.5*candidate_radius;
			}
			/*
			NOTIFY("Projection");
			if(next_parameter[next_parameter.length()-2]<=0) {
				NOTIFY("p is negative");
				next_parameter[next_parameter.length()-2]=0;
			}
			else if(next_parameter[next_parameter.length()-1]<=0) {
				NOTIFY("q is negative");
				next_parameter[next_parameter.length()-1]=0;
			}
			*/
								
			
		}	//while

}



	















