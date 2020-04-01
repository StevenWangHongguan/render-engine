#ifndef PARTICLE_RENDERER_H
#define PARTICLE_RENDERER_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/gtc/matrix_transform.hpp> 
#include <glm/gtc/type_ptr.hpp>

#include "myShader.h"
#include "Particle.h"
#include "Quad.h"

#include <list>
using namespace std;

typedef glm::mat4 mat4;

//����Ϊstatic ֻ���ڱ��ļ���ʹ��
static const unsigned int SCR_WIDTH = 1280;
static const unsigned int SCR_HEIGHT = 720;
static const unsigned int MAX_INSTANCES = 100000; //ÿ�γ������Ƶ���������� ʮ��� �������˰�
static const unsigned int INSTANCE_DATA_LENGTH = 21; //ÿ��ʵ�����������ݳ���
static float *vboData = new float[MAX_INSTANCES * INSTANCE_DATA_LENGTH]; //����ʹ�þ�̬����ᱨ��

class ParticleRenderer{
private:
	Quad *quad;
	myShader *particleShader;
	unsigned int VBO;
	unsigned int pointer; //����ָ��vboData��ÿ��λ�ø�ֵ

	unsigned int createEmptyVBO(int floatCount);
	void setUpAttributes(unsigned int VAO, unsigned int VBO);

	void prepare(); //������ɫ���еĶ�������
	void bindTexture(ParticleTexture texture); //������ɫ���е�������Ԫ ����1��uniform
	void updateModelViewMatrix(vec3 position, float rotation, float scale, mat4 view, float vboData[]);
	void storeMatrixData(mat4 matrix, float data[]);
	void updateTexCoordsInfo(Particle particle, float data[]);
	void updateVBOdata(unsigned int VBO, float data[]);
	void finishRendering();

public:
	ParticleRenderer();
	void setProjection(mat4 projection);
	void render(list<Particle> particles, Camera camera);
};

//���캯�� �����µ�quad��shader ��ÿһ����Ⱦ���ж��Ե�ͶӰ���� 
ParticleRenderer::ParticleRenderer()
{
	quad = new Quad();
	particleShader = new myShader("shader/particleShader/particleShader.vs", "shader/particleShader/particleShader.fs");
	VBO = createEmptyVBO(MAX_INSTANCES * INSTANCE_DATA_LENGTH); //ʵ���Ĵ�VBO ������������ * ÿ�����ӵ��������� * �������͵ĳ��ȣ��ֽ�����
	setUpAttributes(quad->getQuadVAO(), VBO);
	//ʹ��Ĭ�ϵ�ͶӰ����
	mat4 projection = glm::perspective(glm::radians(60.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);
	particleShader->use();
	particleShader->setMat4("projection", projection);
	particleShader->stop();
}

//���ⲿ����ͶӰ����
void ParticleRenderer::setProjection(mat4 projection)
{
	particleShader->setMat4("projection", projection); 
}

//�������ӵ�ģ����ͼ���� �Ա���Ⱦ ����������ʹ�����ӳ�����ȷ
void ParticleRenderer::updateModelViewMatrix(vec3 position, float rotation, float scale, mat4 view, float vboData[])
{
	mat4 model(1.0f);
	model = glm::translate(model, position);

	//���²���Ϊ���� �������յ�modelview�������ת���� �����Ͻ�3x3�ľ���Ϊ��λ����
	float *p = glm::value_ptr(model);
	float *q = glm::value_ptr(view);
	*p = *q;
	*(p + 1) = *(q + 4);
	*(p + 2) = *(q + 8);
	*(p + 4) = *(q + 1);
	*(p + 5) = *(q + 5);
	*(p + 6) = *(q + 9);
	*(p + 8) = *(q + 2);
	*(p + 9) = *(q + 6);
	*(p + 10) = *(q + 10);

	model = glm::rotate(model, glm::radians(rotation), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, vec3(scale));

	mat4 modelView = view * model;  //���� ��Ӧ���ȳ�������ת��
	storeMatrixData(modelView, vboData); //����VBO����
}

//������ɫ�� ��������� ���û�ϡ���Ȼ���
void ParticleRenderer::prepare()
{
	particleShader->use();
	glBindVertexArray(quad->getQuadVAO());
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);
	glEnableVertexAttribArray(4);
	glEnableVertexAttribArray(5);
	glEnableVertexAttribArray(6);
	glEnable(GL_BLEND); 
	glDepthMask(false); 
}

//��prepare�෴
void ParticleRenderer::finishRendering()
{
	glDepthMask(true);
	glDisable(GL_BLEND);
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
	glDisableVertexAttribArray(3);
	glDisableVertexAttribArray(4);
	glDisableVertexAttribArray(5);
	glDisableVertexAttribArray(6);
	glBindVertexArray(0);
	particleShader->stop();
}

void ParticleRenderer::render(list<Particle> particles, Camera camera)
{
	mat4 view = camera.GetViewMatrix();
	ParticleTexture texture = particles.front().getTexture();

	prepare();
	bindTexture(texture);
	
	//float *vboData = new float[particles.size() * INSTANCE_DATA_LENGTH]; //��̬�������� �ᱨ��
	pointer = 0;
	int particlesToDraw = 0; //Ҫ���Ƶ�������
	for (auto &particle : particles)
	{
		if (particle.isAlive())
		{
			updateModelViewMatrix(particle.getPosition(), particle.getRotation(), particle.getScale(), view, vboData);
			updateTexCoordsInfo(particle, vboData);
			particlesToDraw++;
		}
	}
	updateVBOdata(VBO, vboData); //���洢�õ�vbo���ݴ��ݸ�VBO
	glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, quad->getVertexCount(), particlesToDraw);
	
	finishRendering();
}

void ParticleRenderer::bindTexture(ParticleTexture texture)
{
	if (texture.getIsAdditve()) //�Ƿ�ʹ��addtive���
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	else
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glActiveTexture(GL_TEXTURE0); //����������Ԫ ������
	glBindTexture(GL_TEXTURE_2D, texture.getTextureID());
	particleShader->setFloat("numberOfRows", texture.getNumberOfRows());  //������ɫ����uniform
}

//������Ⱦ����ʱ�򴴽�һ���յ�VBO ��Ϊÿһ֡��Ҫ����һ�����ӵ�����ֵ ���Բ���һ��ʼ�ʹ������� ��Ҫÿ�θ������ݸ�VBO ���Կ�ʼ��������ΪNULL ����ʹ��subData����������
unsigned int ParticleRenderer::createEmptyVBO(int floatCount) //���ĸ���������
{
	unsigned int VBO;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, floatCount * sizeof(float), NULL, GL_STREAM_DRAW); //�����õ���stream_draw ��˼Ӧ����ÿ�λ��Ƶ�ʱ�����ݶ���ı� leanOpenGL�̳�����static_drawӦ�������ݴ���һ�� ��ʹ�úܶ�� ���ᱻ�ı�
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	return VBO;
}

//����VAO��VBO������ ����VAO �����Ͼ��е���Щ���� ����ʲô���Ĳ���������ȥ��VBO���ϻ�ȡ�������� Ȼ����θ��¶�������
void ParticleRenderer::setUpAttributes(unsigned int VAO, unsigned int VBO)
{
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBindVertexArray(VAO);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, INSTANCE_DATA_LENGTH * sizeof(float), (void*)(0));
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, INSTANCE_DATA_LENGTH * sizeof(float), (void*)(4 * sizeof(float)));
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, INSTANCE_DATA_LENGTH * sizeof(float), (void*)(8 * sizeof(float)));
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, INSTANCE_DATA_LENGTH * sizeof(float), (void*)(12 * sizeof(float)));
	glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, INSTANCE_DATA_LENGTH * sizeof(float), (void*)(16 * sizeof(float)));
	glVertexAttribPointer(6, 1, GL_FLOAT, GL_FALSE, INSTANCE_DATA_LENGTH * sizeof(float), (void*)(20 * sizeof(float)));
	//����x ÿ��Ⱦһ���µ�ʵ������һ�����ݣ������ƫ�Ƶ���һ��������
	glVertexAttribDivisor(1, 1); 
	glVertexAttribDivisor(2, 1);
	glVertexAttribDivisor(3, 1);
	glVertexAttribDivisor(4, 1);
	glVertexAttribDivisor(5, 1);
	glVertexAttribDivisor(6, 1);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void ParticleRenderer::updateVBOdata(unsigned int VBO, float data[])
{
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, MAX_INSTANCES * INSTANCE_DATA_LENGTH * sizeof(float), data);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void ParticleRenderer::storeMatrixData(mat4 matrix, float data[])
{
	float *pMatrix = glm::value_ptr(matrix);
	for (int i = 0; i < 16; i++)
	{
		*(data + pointer++) = *pMatrix++;  //��data������������� ��Ϊmat4�������ǰ���������� ���Ըպ÷���
	}
}

void ParticleRenderer::updateTexCoordsInfo(Particle particle, float data[])
{
	*(data + pointer++) = particle.getTexOffset1().x;
	*(data + pointer++) = particle.getTexOffset1().y;
	*(data + pointer++) = particle.getTexOffset2().x;
	*(data + pointer++) = particle.getTexOffset2().y;
	*(data + pointer++) = particle.getBlend();
}

#endif