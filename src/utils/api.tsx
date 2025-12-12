import type { AuthPayload, AuthResponse, VideoItem, EditPayload, DeletePayload } from './types';
import axios, { type AxiosInstance } from 'axios';

const axiosInstance: AxiosInstance = axios.create({
    baseURL: '/',
    headers: {
        'Content-Type': 'application/json',
    },
    withCredentials: true,
});

function getAuthHeader(): { Authorization?: string } {
    const name = 'access_token=';
    const decodedCookie = decodeURIComponent(document.cookie);
    const ca = decodedCookie.split(';');

    for (let i = 0; i < ca.length; i++) {
        let c = ca[i];
        c = c.trim();
        if (c.indexOf(name) === 0) {
            const token = c.substring(name.length, c.length);
            if (token.length > 0) {
                return {
                    'Authorization': `Bearer ${token}`
                };
            }
        }
    }
    return {};
}

export async function signUpApi(payload: AuthPayload): Promise<AuthResponse> {
    try {
        const response = await axiosInstance.post('/auth/signup', payload);

        if (response.status === 201 || response.status === 200) {
            return { success: true, message: '회원가입 성공' };
        }
        return { success: false, message: '회원가입 실패: 예상치 못한 응답' };

    } catch (error) {
        const errorMessage = (error as any).response?.data || (error as any).message || '네트워크 오류';
        return { success: false, message: `회원가입 실패: ${errorMessage}` };
    }
}

export async function signInApi(payload: AuthPayload): Promise<AuthResponse> {
    try {
        const response = await axiosInstance.post('/auth/login', payload);

        if (response.status === 200 && response.data.token) {
            const token: string = response.data.token;

            document.cookie = `access_token=${token}; path=/; max-age=36000; SameSite=Lax`;

            return { success: true, message: '로그인 성공', token: token };
        }

        return { success: false, message: '로그인 실패: 응답 형식 오류' };

    } catch (error) {
        const errorMessage = (error as any).response?.data || (error as any).message || '네트워크 오류';
        return { success: false, message: `로그인 실패: ${errorMessage}` };
    }
}

export async function fetchVideoList(): Promise<VideoItem[]> {
    try {
        const response = await axiosInstance.get('/video/list');

        if (response.status === 200) {
            return response.data;
        }
        return [];
    } catch (error) {
        console.error('영상 목록 로드 실패:', error);
        return [];
    }
}

export async function getProgress(videoId: number, token: string | null): Promise<number> {
    if (!token) { return 0; }

    const headers = { 'Authorization': `Bearer ${token}` };

    try {
        const response = await axiosInstance.get(`/video/progress?id=${videoId}`, { headers });
        if (response.status === 200) {
            return response.data.last_position || 0;
        }
        return 0;
    } catch (error) {
        console.error("Failed to get progress (401 expected if not logged in):", error);
        return 0;
    }
}

export async function saveProgress(videoId: number, position: number, token: string | null): Promise<void> {
    if (!token) { return; }

    const headers = { 'Authorization': `Bearer ${token}`, 'Content-Type': 'application/json' };

    try {
        await axiosInstance.post('/video/progress', {
            video_id: videoId,
            position: Math.floor(position)
        }, { headers });

    } catch (error) {
        console.error("Failed to save progress:", error);
    }
}

export async function editVideo(payload: EditPayload, token: string | null): Promise<AuthResponse> {
    if (!token) { return { success: false, message: "인증 토큰이 없습니다." }; }

    const headers = { 'Authorization': `Bearer ${token}`, 'Content-Type': 'application/json' };

    try {
        const response = await axiosInstance.post('/video/edit', payload, { headers });

        if (response.status === 200) {
            return { success: true, message: "영상 정보가 수정되었습니다." };
        }
        return { success: false, message: response.data?.error || "수정 실패" };

    } catch (error) {
        const msg = (error as any).response?.data || (error as any).message;
        return { success: false, message: `수정 중 오류: ${msg}` };
    }
}

export async function deleteVideo(videoId: number, token: string | null): Promise<AuthResponse> {
    if (!token) { return { success: false, message: "인증 토큰이 없습니다." }; }

    const headers = { 'Authorization': `Bearer ${token}` };

    try {
        const response = await axiosInstance.delete(`/video/delete?id=${videoId}`, { headers });

        if (response.status === 200) {
            return { success: true, message: "영상이 삭제되었습니다." };
        }
        return { success: false, message: response.data?.error || "삭제 실패" };
    } catch (error) {
        const msg = (error as any).response?.data || (error as any).message;
        return { success: false, message: `삭제 중 오류: ${msg}` };
    }
}

export async function uploadVideo(file: File, title: string, token: string | null): Promise<AuthResponse> {
    if (!token) { return { success: false, message: "인증 토큰이 없습니다." }; }

    if (file.type !== 'video/mp4') {
        return { success: false, message: "오직 MP4 파일만 업로드할 수 있습니다." };
    }

    const headers = {
        'Authorization': `Bearer ${token}`,
        // 'Content-Type': 'video/mp4',
        // 'X-Video-Title': encodeURIComponent(title)
    };

    const url = `/video/upload`;

    try {
        console.log(headers)

        const response = await axiosInstance.post(url, file, {
            headers,
            // maxBodyLength: Infinity,
            // maxContentLength: Infinity,
        });

        console.log('done')

        console.log(response)

        if (response.status === 200) {
            return { success: true, message: "파일 업로드 및 처리 성공" };
        }

        return { success: false, message: response.data?.error || "업로드 실패" };

    } catch (error) {
        const msg = (error as any).response?.data || (error as any).message;
        return { success: false, message: `업로드 중 오류: ${msg}` };
    }
}
